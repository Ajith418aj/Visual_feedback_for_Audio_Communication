import socket, simpleaudio as sa
import threading
from multiplex import recv, send

class ServerTCP:
    
    def __init__(self):
            while 1:
                try:
                    self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                    self.s.bind(('0.0.0.0', 0))
                    self.close = 0
                    break
                except:
                    print("Couldn't bind to that port tcp")

            self.connections = []

    def get_ports(self): #get the port number on which this server is running
        return self.s.getsockname()

    def accept_connections(self): #Function that always looks for a new client
        self.s.listen(100)
        while True:
            conn, addr = self.s.accept()

            print("Connection" + str(conn) + "\n" + str(addr))

            self.connections.append(conn)
            threading.Thread(target=self.handle_client,args=(conn,addr,)).start() #separate thread for each client
            if self.close:
                break
        
    def broadcast(self, sock, data): #Function used for broadcasting message from a client to others
        for client in self.connections:
            if client != self.s and client != sock:
                try:
                    send(client, data, encode= False)
                except:
                    pass

    def remove(self, conn):
        if conn in self.connections: 
            self.connections.remove(conn) 
            if len(self.connections) == 0:
                self.close = 1

    def handle_client(self,conn,addr): #Receives data from a client and calls broadcast
        while 1:
            try:
        
                data = recv(conn, decode=False) #Look for recv funtion in multiplex.py, that can also be used but required are to be done in android code as well because recv function required first 4 bytes to be the length of the packet
                # print("Read : " + str(len(data)))

                if data:
                    self.broadcast(conn, data)
                else:
                    self.remove(conn)
                    conn.close()
                    break
            except:
                # print("Exception Occured")
                continue


    def run(self): #Calls accept_connections which always looks for a new connection
        self.accept_connections()

