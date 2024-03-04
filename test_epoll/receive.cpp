#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <iostream>
#include <vector>
#include <iostream> 
#include <fstream>
using namespace std;

int main()
{

    // Configure the receiver address and port to forward the data
    const char *receiverIP = "localhost"; // Replace with the actual receiver IP (client2)
    int receiverPort = 12345;             // Replace with the actual receiver port

    // Set up the receiver address
    sockaddr_in receiverAddress{};
    receiverAddress.sin_family = AF_INET;
    receiverAddress.sin_port = htons(receiverPort);
    if (inet_pton(AF_INET, receiverIP, &(receiverAddress.sin_addr)) < 0)
    {
        perror("Failed to set up receiver address");
        return 1;
    }

    int receivingSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (receivingSocket < 0)
    {
        perror("Failed to create socket");
        return 1;
    }

    if (bind(receivingSocket, (struct sockaddr *)&receiverAddress, sizeof(receiverAddress)) < 0)
    {
        perror("Failed to bind forwarding socket");
        return 1;
    }

    // Create the epoll instance
    int epollInstance = epoll_create1(0);
    if (epollInstance == -1)
    {
        perror("Failed to create epoll instance");
        return 1;
    }

    // Register the forwarding socket for read events
    epoll_event event{};
    event.events = EPOLLIN;
    event.data.fd = receivingSocket;
    if (epoll_ctl(epollInstance, EPOLL_CTL_ADD, receivingSocket, &event) == -1)
    {
        perror("Failed to register forwarding socket with epoll");
        return 1;
    }

    // Create the events vector to store the events returned by epoll
    const int maxEvents = 10;
    std::vector<epoll_event> events(maxEvents);

    // Forward data received from clients to the receiver using epoll
    char buffer[1024];
    ofstream MyFile("frcv.pcm");
    while (true)
    {
        int numEvents = epoll_wait(epollInstance, events.data(), maxEvents, -1);
        if (numEvents == -1)
        {
            perror("Failed to wait for events using epoll");
            return 1;
        }

        for (int i = 0; i < numEvents; ++i)
        {
            // Handle events for the forwarding socket
            if (events[i].data.fd == receivingSocket)
            {
                sockaddr_in clientAddress{};
                socklen_t clientAddressLength = sizeof(clientAddress);
                ssize_t bytesRead;
                while (1) // so that the buffer can be read continuously
                {

                    ssize_t bytesRead = recvfrom(receivingSocket, buffer,  sizeof(buffer), 0, (struct sockaddr *)&clientAddress, &clientAddressLength);
                    MyFile<<buffer;
                    if (bytesRead <= 0)
                    {
                        perror("Failed to receive data from client");
                        break;
                    }
                }
                // buffer[bytesRead]='\0';
            }
        }
    }

    // Close the forwarding socket and epoll instance
    close(receivingSocket);
    close(epollInstance);

    return 0;
}