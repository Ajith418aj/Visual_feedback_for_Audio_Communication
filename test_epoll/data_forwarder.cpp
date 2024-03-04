#include <iostream>
#include <cstring>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include "nlohmann/json.hpp"
#include <queue>
#include <thread>
#include <mutex>
#include <arpa/inet.h>
#define PORT 5000
#define MAX_BUFFER_SIZE 1024
std::mutex queueMutex;
using namespace std;

const int MAX_EVENTS = 100;
int audio_socket, ack_socket;     // for audio and ack sockets
int audio_port = 0, ack_port = 0; // initialization for audio and ack ports
int server_fd, new_socket, valread;
std::queue<std::pair<std::string, std::string>> dataQueue;
std::vector<std::string> clients; // class can be created for this, this is used to store the clients in a room
struct sockaddr_in address;
int opt = 1;
int addrlen = sizeof(address);
char buffer[1024] = {0};
struct epoll_event event;
struct epoll_event events[MAX_EVENTS];

map<int, int> socket_type;
void receiveData(int audio_socket, int ack_socket, std::queue<std::pair<std::string, std::string>> &dataQueue)
{
    char audio_buffer[MAX_BUFFER_SIZE] = {0};

    struct sockaddr_in senderAddress
    {
    };
    socklen_t senderAddrLen = sizeof(senderAddress); // client socket length ?

    while (true)
    {
        // Receive data from audio_socket
        int audio_valread = recvfrom(audio_socket, audio_buffer, sizeof(audio_buffer), 0, reinterpret_cast<struct sockaddr *>(&senderAddress), &senderAddrLen);
        if (audio_valread > 0)
        {
            // Retrieve the IP address of the sender
            char senderIP[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(senderAddress.sin_addr), senderIP, INET_ADDRSTRLEN);

            // Convert received data to a string
            std::string audio_data(audio_buffer, audio_valread);

            // Store the IP address and data as a pair in the queue
            std::pair<std::string, std::string> audio_packet(senderIP, audio_data);

            std::lock_guard<std::mutex> lock(queueMutex);
            dataQueue.push(audio_packet);
        }
        else
        {
            // Handle error or connection closed
            break;
        }
    }
}
void handleUDPSocketEvent(int audioSocket,int destport)
{
    char buffer[1024];
    ssize_t bytesRead = read(audioSocket, buffer, 1024);

    if (bytesRead > 0)
    {
        ssize_t bytesWritten = write(destport, buffer, bytesRead);

        if (bytesWritten == bytesRead)
        {
            cout<<"Data successfully written";
            // All the data was successfully written to the destination file descriptor
        }
        else
        {
            cout<<"Error occured while writing data";
            cerr << "Failed to write file: " <<strerror(errno) <<endl;
           
        }
    }
    else if (bytesRead == 0)
    {
        cout<<"Reached end of file";
    }
    else
    {
        cerr << "Failed to read file: " <<strerror(errno) <<endl;
    }
}
int main()
{
    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1)
    {
        std::cerr << "Failed to create epoll instance." << std::endl;
        return -1;
    }

    // Create a UDP socket
    int server_fd;
    char buffer[1024];
    struct sockaddr_in servaddr, cliaddr;
       
    // Creating socket file descriptor
    if ( (server_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
       
    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));
       
    // Filling server information
    servaddr.sin_family    = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);
       
    // Bind the socket with the server address
    if ( bind(server_fd, (const struct sockaddr *)&servaddr, 
            sizeof(servaddr)) < 0 )
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
       
    socklen_t len;
    int n;
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = server_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event) == -1)
    {
        cerr << "Failed to register TCP socket with epoll." << endl;
        close(server_fd);
        close(epoll_fd);
        return -1;
    }
    int destport = 12345;
    int num_events;
    while (true)
    {
        num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (num_events == -1)
        {
            cerr << "Failed to wait for events" << endl;
            break;
        }
        for (int i = 0; i < num_events; ++i)
        {
            if (events[i].data.fd == server_fd)
            {
                handleUDPSocketEvent(server_fd,destport);
            }
            else
            {
               // do_nothing
            }
        }
    }
    close(new_socket);
    close(server_fd);

    return 0;
}
