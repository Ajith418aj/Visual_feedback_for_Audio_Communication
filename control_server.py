import collections, json, selectors, socket, _thread, socketserver, simpleaudio as sa
from errno import ENODEV
from threading import Thread
from multiplex import recv, send
from queue import Queue
import requests
from http.server import BaseHTTPRequestHandler, HTTPServer
import time
import socket
import struct
NUM_PARTICIPANTS = 256
 
 
 
class TCPMainServer(socketserver.BaseRequestHandler):
    table = {} # this table will contain following columns (Room_id, IP_Port, Password, Ports_info) with Room_id as key and other as values
 
    def clear_room(self, room_id): #This function will clear the room id and password
        self.table.pop(room_id)
 
    def IPC(self, client_info): #This function deals with the inter process communication between control server and data forwarder server
        queue = Queue()
 
        ports = {}
        self.s = socket.socket()
        port = 5000  # Port where the data forwarder is listening
        print("IPC_client_info =", client_info)
        self.s.connect(('localhost', port))
        client_info_str = json.dumps(client_info)
        self.s.send(client_info_str.encode('utf-8'))  # Sends client info to data forwarder
 
        # Receive the response from the data forwarder
        response_str = self.s.recv(1024).decode('utf-8')
 
        response = json.loads(response_str)
        #print("Response type: ", type(response))
        print("Response from data forwarder: ", response)
 
 
        # Extract the Room_id from the response
        # EndMeeting = response["Room_id"]
        # print("Room id received:", EndMeeting) # this is the room id of which the meeting has ended
        # self.clear_room(EndMeeting)
        # print("Active rooms after clearing =", self.table)
 
 
        #Now extract the audio port and ack port
        audio_port = response["audio_port"]
        ack_port = response["ack_port"]
        rr_port = response ["rr_port"]
 
        #convert the ports(string) to int
        audio_port = int(audio_port)
        ack_port = int(ack_port)
        rr_port = int(rr_port)
 
        #convert to bytes
        audio_port_byte = audio_port.to_bytes(4,'big')
        ack_port_byte = ack_port.to_bytes(4,'big')
        rr_port_byte = rr_port.to_bytes(4,'big')
 
 
        #return audio_port, ack_port #include rr_port too
        ports_info = audio_port_byte + ack_port_byte + rr_port_byte
        print("ports_info: ", ports_info)
        return ports_info
 
 
 
        # Close the socket
        self.s.close()
 
 
        # and clear the room_id and password from "room"
 
    def handle(self):
        print("hi2")
        # self.request is the TCP socket connected to the client
        self.data = recv(self.request, decode= False).decode("utf-8") # receiving request from client. Either "join" or "create"
        data = self.data.split()
        roomName = data[1]
        passwd = data[2]
        #print(data)
        self.addr = self.client_address
        print("Client addr:", self.addr)
 
        if data[0] == "Create": #Check if the request is create
            #check if room_id and pswd already exist
            if roomName in self.table.keys() and self.table [roomName][0][1] == passwd:
                port = 65536
                port_byte = port.to_bytes(4, 'big')
                print("Room already exists")
                send(self.request, port_byte, encode= False) 
            else:
                self.table[roomName] = self.client_address #map the room_id along with it's host (IP and port)
 
                self.table[roomName] = [self.table[roomName], passwd] #add password to table
 
                print("Active rooms = ", self.table)
 
                #send client's info to data forwarder by calling the IPC...for this create a new thread
                client_info = data + list(self.addr) #client_info = ['Request type', 'room id', 'password', 'client ip', client port]
                #print("Client_info= ", client_info)
 
                # Here threading is being removed because it may cause race condition, can be handled but 
                # implementation will become complex      
                ports_info = self.IPC(client_info) 
                 # ports_info:  ('audio', 'ack', 'port')....ports_info:  ('52602', '46326', '45666')
                #Add the ports_info to the table
                self.table[roomName] = [self.table[roomName], ports_info]    
                print("type of ports_info: ", type(ports_info))
 
                #send the ports info
 
                # data = packed_data.encode('utf-8')
                send(self.request, ports_info, encode=False)
 
 
 
 
        elif data[0] == "Join":   #Check if it is join request
 
            print("Join Request for: ", roomName)
            if roomName in self.table.keys() and self.table[roomName][0][1] == passwd:    #Check if room exist
 
                #send client's info to data forwarder by calling the IPC...for this create a new thread
                client_info = roomName #this will not handle same room_name but different password (in the data forwarder side)
 
                ports_info = self.table[roomName][1]  
                data = ports_info.encode('utf-8')
                send(self.request, data, encode=False)
 
            else:
                port = 65536   #Send a wrong port number specifying that room non-existing or passwd wrong.
                port_byte = port.to_bytes(4, 'big')
                send(self.request, port_byte, encode= False)
 
        elif data[0] == "End call": #room_id needs to be send too when pressing end call button
            #check if room is active
            if roomName in self.table.keys():
                client_info = roomName #client info = ['room id']
            else:
                print("Room not active")
            #check if the client is the host
            ip_port = self.client_address #extract the ip and port
            if ip_port == self.table[roomName][0][0]: #check if the ip and port matches with the one stored in table
                 #if it is a host forward to data forwarder
                self.IPC(client_info)
 
            #else send error 
            print("You are not the host") #temp line
 
 
 
 
 
 
 
if __name__ == "__main__":
    HOST, PORT = "10.129.131.216", 8086                                          #Server address and port
    hostName = "localhost"
 
 
    # Create the server, binding to localhost on port 9998
    with socketserver.TCPServer((HOST, PORT), TCPMainServer) as server:  
        print("hi")   
 
        # Activate the server; this will keep running until you
        # interrupt the program with Ctrl-C
        try:
            server.serve_forever()												  #Run this main server forever.
        except KeyboardInterrupt:
 
            print("process terminated")
            pass
 
        print("hi2")
        exit()
 
        # return