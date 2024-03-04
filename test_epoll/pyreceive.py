import socket
def recv_pcm_file_from_server(file_path, host, port):
    buffer = ''

    while True:
        data, addr = udp_socket.recv(1024)
        if data:
            buffer += data
        else:
            break
        
    with open('received.txt', 'wb') as file:
        file.write(buffer)


# Create a UDP socket
udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# Server address and port
server_host = 'localhost'
server_port = 12345

# Bind the socket to the specified port
udp_socket.bind((server_host, server_port))
recv_pcm_file_from_server("sample.txt", server_host, server_port)

