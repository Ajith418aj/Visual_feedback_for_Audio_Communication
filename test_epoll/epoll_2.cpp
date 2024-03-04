#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <iostream>
#include <vector>
#include <unordered_map>

int main() {
    // Create a socket for the data forwarding server
    int forwardingSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (forwardingSocket < 0) {
        perror("Failed to create socket");
        return 1;
    }

    // Configure the server address and port to forward the data
    const char* serverIP = "127.0.0.1";  // Replace with the actual server IP
    int serverPort = 12345;  // Replace with the actual server port

    // Set up the server address
    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(serverPort);
    if (inet_pton(AF_INET, serverIP, &(serverAddress.sin_addr)) <= 0) {
        perror("Failed to set up server address");
        return 1;
    }

    // Bind the forwarding socket to a specific port
    sockaddr_in forwardingAddress{};
    forwardingAddress.sin_family = AF_INET;
    forwardingAddress.sin_addr.s_addr = INADDR_ANY;
    forwardingAddress.sin_port = htons(8888);  // Replace with the desired forwarding port

    if (bind(forwardingSocket, (struct sockaddr*)&forwardingAddress, sizeof(forwardingAddress)) < 0) {
        perror("Failed to bind forwarding socket");
        return 1;
    }

    // Create the epoll instance
    int epollInstance = epoll_create1(0);
    if (epollInstance == -1) {
        perror("Failed to create epoll instance");
        return 1;
    }

    // Register the forwarding socket for read events
    epoll_event event{};
    event.events = EPOLLIN;
    event.data.fd = forwardingSocket;
    if (epoll_ctl(epollInstance, EPOLL_CTL_ADD, forwardingSocket, &event) == -1) {
        perror("Failed to register forwarding socket with epoll");
        return 1;
    }

    // Create the events vector to store the events returned by epoll
    const int maxEvents = 10;
    std::vector<epoll_event> events(maxEvents);

    // Create a map to store client sockets
    std::unordered_map<int, sockaddr_in> clientSockets;

    // Forward data received from the sending client to the receiving client using epoll
    char buffer[1024];
    while (true) {
        int numEvents = epoll_wait(epollInstance, events.data(), maxEvents, -1);
        if (numEvents == -1) {
            perror("Failed to wait for events using epoll");
            return 1;
        }

        for (int i = 0; i < numEvents; ++i) {
            // Handle events for the forwarding socket
            if (events[i].data.fd == forwardingSocket) {
                sockaddr_in clientAddress{};
                socklen_t clientAddressLength = sizeof(clientAddress);
                ssize_t bytesRead = recvfrom(forwardingSocket, buffer, sizeof(buffer), 0,
                                             (struct sockaddr*)&clientAddress, &clientAddressLength);
                if (bytesRead < 0) {
                    perror("Failed to receive data from client");
                    return 1;
                }

                // Check if the client is the sender or receiver
                // Check if the client is the sender or receiver
                int clientSocket = events[i].data.fd;
                if (clientSockets.find(clientSocket) == clientSockets.end()) {
                    // Add sender client socket to the map
                    clientSockets[clientSocket] = clientAddress;
                } else {
                    // Forward the received data to the receiver client
                    ssize_t bytesSent = sendto(forwardingSocket, buffer, bytesRead, 0,
                                               (struct sockaddr*)&clientSockets[clientSocket], sizeof(clientSockets[clientSocket]));
                    if (bytesSent < 0) {
                        perror("Failed to send data to client");
                        return 1;
                    }
                }
            } else {
                // Handle events for individual client sockets
                // (if you have specific operations for each client, you can handle them here)
            }
        }
    }

    // Close the forwarding socket and epoll instance
    close(forwardingSocket);
    close(epollInstance);

    return 0;
}
