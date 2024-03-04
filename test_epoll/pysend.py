import socket
import time

def send_pcm_file_to_server(file_path, host, port):
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    client_socket.connect((host, port))
    delay = 5/10000
    with open(file_path, 'rb') as file:
        itr=0
        while True:
            time.sleep(delay)
            chunk = file.read(4096)
            if not chunk:
                break
            client_socket.send(chunk)
            itr=itr+1
            print("sending chunk id : ",itr)

    client_socket.close()

if __name__ == "__main__":
    host = "localhost"
    port = 8000
    file_path = "send.pcm"
    send_pcm_file_to_server(file_path, host, port)
    
#create a delay
# error check at every system call.