#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <iostream>
#include <vector>

#include <iostream>
using namespace std;

int main()
{
    // Create a socket for the data forwarding receiver
    int forwardingSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (forwardingSocket < 0)
    {
        perror("Failed to create socket");
        return 1;
    }

    
    const char *receiverIP = "localhost";
    int receiverPort = 12345;             

    // Set up the receiver address
    sockaddr_in receiverAddress{};
    receiverAddress.sin_family = AF_INET;
    receiverAddress.sin_port = htons(receiverPort);
    if (inet_pton(AF_INET, receiverIP, &(receiverAddress.sin_addr)) < 0)
    {
        perror("Failed to set up receiver address");
        return 1;
    }

    // Bind the forwarding socket to a specific port
    sockaddr_in forwardingAddress{};
    forwardingAddress.sin_family = AF_INET;
    forwardingAddress.sin_addr.s_addr = INADDR_ANY;
    forwardingAddress.sin_port = htons(8000); 
    if (bind(forwardingSocket, (struct sockaddr *)&forwardingAddress, sizeof(forwardingAddress)) < 0)
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
    event.data.fd = forwardingSocket;
    if (epoll_ctl(epollInstance, EPOLL_CTL_ADD, forwardingSocket, &event) == -1)
    {
        perror("Failed to register forwarding socket with epoll");
        return 1;
    }

    // Create the events vector to store the events returned by epoll
    const int maxEvents = 10;
    std::vector<epoll_event> events(maxEvents);

    // Forward data received from clients to the receiver using epoll
    char buffer[100000];
    long long total_bytes_read = 0;
    int count=0;
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
            if (events[i].data.fd == forwardingSocket)
            {
                sockaddr_in clientAddress{};
                socklen_t clientAddressLength = sizeof(clientAddress);
                while (1) // so that the buffer can be read continuously
                {
                    ssize_t bytesRead = recvfrom(forwardingSocket, buffer, sizeof(buffer), 0, (struct sockaddr *)&clientAddress, &clientAddressLength);
                    total_bytes_read+=bytesRead;
                    if (bytesRead <= 0)
                    {
                        perror("Failed to receive data from client");
                        break;
                    }
                    cout<<"forwarding chunk id : "<<" "<<count++<<endl;
                    while (bytesRead > 0)
                    {
                        ssize_t bytesSent = sendto(forwardingSocket, buffer, bytesRead, 0, (struct sockaddr *)&receiverAddress, sizeof(receiverAddress));
                        if (bytesSent < 0)
                        {
                            perror("Failed to send data to receiver");
                            return 1;
                        }
                        bytesRead-=bytesSent;
                    }
                }
            }
        }
        //cout<<total_bytes_read<<endl;
    }

    // Close the forwarding socket and epoll instance
    close(forwardingSocket);
    close(epollInstance);

    return 0;
}