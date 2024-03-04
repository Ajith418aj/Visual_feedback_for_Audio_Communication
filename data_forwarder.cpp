#include <iostream>
#include <cstring>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "nlohmann/json.hpp"
#include <queue>
#include <thread>
#include <mutex>
#include <arpa/inet.h>

#define PORT 5000
#define MAX_BUFFER_SIZE 1024
std::mutex queueMutex;
using namespace std;

// This function receives the data from audio and ack socket and stores it in queue
// the type of queue is pair because we need to store the sender's ip along with it's data
// which is further needed in forwarding function

void receiveData(int audio_socket, int ack_socket, std::queue<std::pair<std::string, std::string>>& dataQueue) {
    char audio_buffer[MAX_BUFFER_SIZE] = {0};
    char ack_buffer[MAX_BUFFER_SIZE] = {0};

    struct sockaddr_in senderAddress{};
    socklen_t senderAddrLen = sizeof(senderAddress); //client socket length ?

    while (true) {
        // Receive data from audio_socket
        int audio_valread = recvfrom(audio_socket, audio_buffer, sizeof(audio_buffer), 0, reinterpret_cast<struct sockaddr*>(&senderAddress), &senderAddrLen);
        if (audio_valread > 0) {
            // Retrieve the IP address of the sender
            char senderIP[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(senderAddress.sin_addr), senderIP, INET_ADDRSTRLEN);

            // Convert received data to a string
            std::string audio_data(audio_buffer, audio_valread);

            // Store the IP address and data as a pair in the queue
            std::pair<std::string, std::string> audio_packet(senderIP, audio_data);

            std::lock_guard<std::mutex> lock(queueMutex);
            dataQueue.push(audio_packet);
        } else {
            // Handle error or connection closed
            break;
        }

        // Receive data from ack_socket
        int ack_valread = recvfrom(ack_socket, ack_buffer, sizeof(ack_buffer), 0, reinterpret_cast<struct sockaddr*>(&senderAddress), &senderAddrLen);
        if (ack_valread > 0) {
            // Retrieve the IP address of the sender
            char senderIP[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(senderAddress.sin_addr), senderIP, INET_ADDRSTRLEN);

            // Convert received data to a string
            std::string ack_data(ack_buffer, ack_valread);

            // Store the IP address and data as a pair in the queue
            std::pair<std::string, std::string> ack_packet(senderIP, ack_data);

            std::lock_guard<std::mutex> lock(queueMutex);
            dataQueue.push(ack_packet);
        } else {
            // Handle error or connection closed
            break;
        }
    }
}

//----------------------------------------------------------------------------------------------------
void forwardData(std::queue<std::pair<std::string, std::string>>& dataQueue, const std::vector<std::string>& clients) {
    while (true) {
        std::pair<std::string, std::string> packet;
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            if (!dataQueue.empty()) {
                packet = dataQueue.front();
                dataQueue.pop();
            } else {
                // No more data in the queue, exit the function
                return;
            }
        }

        const std::string& senderIP = packet.first;
        const std::string& data = packet.second;

        for (const auto& clientIP : clients) {
            if (clientIP != senderIP) { //this is to send data to other clients in the room except itself
                // Forward the data to the client IP address
                std::cout << "Forwarding data to " << clientIP << std::endl;

                // Create a socket to send data
                int forwardSocket = socket(AF_INET, SOCK_DGRAM, 0);
                if (forwardSocket == -1) {
                    std::cerr << "Failed to create forwarding socket" << std::endl;
                    return;
                }

                // Set up the socket address for the clients
                struct sockaddr_in clientAddress{};
                clientAddress.sin_family = AF_INET;
                clientAddress.sin_port = htons(20002);  // Replace with the appropriate port
                if (inet_pton(AF_INET, clientIP.c_str(), &(clientAddress.sin_addr)) <= 0) {
                    std::cerr << "Invalid client IP address" << std::endl;
                    close(forwardSocket);
                    return;
                }

                // Forward the data to the clients
                ssize_t bytesSent = sendto(forwardSocket, data.c_str(), data.length(), 0,
                                           reinterpret_cast<struct sockaddr*>(&clientAddress), sizeof(clientAddress));
                if (bytesSent == -1) {
                    std::cerr << "Failed to forward data to " << clientIP << std::endl;
                    close(forwardSocket);
                    return;
                }

                close(forwardSocket);
            }
        }
    }
}
//----------------------------------------------------------------------------------------------------

int main() {
    std::queue<std::pair<std::string, std::string>> dataQueue;
    std::vector<std::string> clients; // class can be created for this, this is used to store the clients in a room

    int server_fd, new_socket, valread;
    int audio_socket, ack_socket; // for audio and ack sockets
    int audio_port = 0, ack_port = 0; // initialization for audio and ack ports
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};

    // Create a TCP socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options to reuse address and port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket to localhost and port 5000
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 1) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    // Accept a new connection
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
        perror("accept failed");
        exit(EXIT_FAILURE);
    }

    // Receive data from the client
    valread = recv(new_socket, buffer, sizeof(buffer), 0);

    // Convert received data to a string
    string received_data(buffer, valread);
    cout << "Received data: " << received_data << endl; // For debugging

    // Parse the JSON string into a list
    using json = nlohmann::json;
    json client_info;

    try {
        client_info = json::parse(received_data);
    } catch (const json::exception& e) {
        cerr << "Error parsing JSON: " << e.what() << endl;
        // Handle the error gracefully
        // ...
        return 1; // Or return an appropriate error code
    }

    // Send the response back to the client
    //send(new_socket, response.c_str(), response.length(), 0);
    
    // Access the values in the client_info list
    string ReqType = client_info[0]; //request type
    string Room_id = client_info[1]; 
    string Pwd = client_info[2];

    // Extract the IP address and port
    string ip_address = client_info[3];
   // string port = client_info[4];
   clients.push_back(ip_address);

    // Iterating over the list
    std::cout << "Clients in a room: ";
    for (const auto& element : clients) {
        std::cout << element << " ";
    }
    std::cout << std::endl;

    // Prepare the response based on the received values
    //------------------------------------------------------------------------------------------------------------------------------
    json response;
    if (ReqType == "Create") 
    {
        
        struct sockaddr_in udp_address1, udp_address2;
        

        // Create the first UDP socket
        if ((audio_socket = socket(AF_INET, SOCK_DGRAM, 0)) == 0) {
            perror("socket failed");
            exit(EXIT_FAILURE);
        }

        // Create the second UDP socket
        if ((ack_socket = socket(AF_INET, SOCK_DGRAM, 0)) == 0) {
            perror("socket failed");
            exit(EXIT_FAILURE);
        }

        // Set socket options to reuse address and port
        if (setsockopt(audio_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
            perror("setsockopt failed");
            exit(EXIT_FAILURE);
        }

        if (setsockopt(ack_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
            perror("setsockopt failed");
            exit(EXIT_FAILURE);
        }

        udp_address1.sin_family = AF_INET;
        udp_address1.sin_addr.s_addr = INADDR_ANY;
        udp_address1.sin_port = 0; // Automatically choose a free port

        udp_address2.sin_family = AF_INET;
        udp_address2.sin_addr.s_addr = INADDR_ANY;
        udp_address2.sin_port = 0; // Automatically choose a free port

        // Bind the first UDP socket
        if (bind(audio_socket, (struct sockaddr *)&udp_address1, sizeof(udp_address1)) < 0) {
            perror("bind failed");
            exit(EXIT_FAILURE);
        }

        // Bind the second UDP socket
        if (bind(ack_socket, (struct sockaddr *)&udp_address2, sizeof(udp_address2)) < 0) {
            perror("bind failed");
            exit(EXIT_FAILURE);
        }

        // Get the port numbers of the UDP sockets
        socklen_t len1 = sizeof(udp_address1);
        socklen_t len2 = sizeof(udp_address2);
        if (getsockname(audio_socket, (struct sockaddr *)&udp_address1, &len1) == -1) {
            perror("getsockname failed");
            exit(EXIT_FAILURE);
        }
        if (getsockname(ack_socket, (struct sockaddr *)&udp_address2, &len2) == -1) {
            perror("getsockname failed");
            exit(EXIT_FAILURE);
        }

        audio_port = ntohs(udp_address1.sin_port);
        ack_port = ntohs(udp_address2.sin_port);

        // Store the UDP port numbers and room id in the response
        response["Room_id"] = Room_id;
        response["audio_port"] = std::to_string(audio_port); //converting audio port to string
        response["ack_port"] = std::to_string(ack_port);

       
    }

    //------------------------------------------------------------------------------------------------------------------------------

    std::cout << "Response in json: "  << response << std::endl;

    // Start receiving data from audio_socket and ack_socket
    // Start a new thread to receive data from the socket

    std::thread receive_thread([&]() {
        receiveData(audio_socket, ack_socket, std::ref(dataQueue));
    });

    std::thread forwardingThread(forwardData, std::ref(dataQueue), clients);



    // Send the response back to the control server
    std::string response_str = response.dump();  // Convert response to std::string

    // Remove whitespace from the response string
    response_str.erase(std::remove(response_str.begin(), response_str.end(), ' '), response_str.end());

    send(new_socket, response_str.c_str(), response_str.length(), 0);

// Close the TCP socket
close(new_socket);
close(server_fd);

return 0;
}
