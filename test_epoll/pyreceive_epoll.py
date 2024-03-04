import socket
import select

UDP_IP = '127.0.0.1'  # IP address of the server
UDP_PORT = 12345     # Port number to listen on
BUFFER_SIZE = 10000   # Size of the receive buffer
OUTPUT_FILE = 'output.pcm'  # Name of the output file

# Create a UDP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

epoll = select.epoll()
epoll.register(sock.fileno(), select.EPOLLIN)

try:
    with open(OUTPUT_FILE, 'wb') as f:
        count = 0
        while True:
            bytes_read=0
            events = epoll.poll()  # Wait for events
            
            for fileno, event in events:
                if fileno == sock.fileno():
                    data, addr = sock.recvfrom(BUFFER_SIZE)
                    if not data:
                        # The server has finished sending data
                        print("Server finished sending data")
                        break
                    f.write(data)
                    #bytes_read=bytes_read+len(data)
                    print("Receiving chunk id : ",count)
                    count = count +1
            # print(bytes_read)

finally:
    # Cleanup
    epoll.unregister(sock.fileno())
    epoll.close()
    sock.close()