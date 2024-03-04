import socket, simpleaudio as sa
import threading, queue
import time
from multiplex import *

obj = threading.Semaphore(0)

class ServerUDP:
	def __init__(self):
		
		while 1:
			try:
				self.s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
				self.s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
				self.s.bind(('0.0.0.0', 20002))
				self.clients = set()
				# new code
				self.client_recv_portmaps = {}
				self.recvPackets = queue.Queue()
				break
			except:
				print("Couldn't bind to that port udp")

	def get_ports(self):
		return self.s.getsockname()

	def RecvData(self):
		while True:
			data,addr = self.s.recvfrom(3600)
			# print("addr is: ", addr)
			# print("Type of addr is: ", type(addr))
			y = list(addr)
			
			print("Recved data",len(data))
			if(data == "Port map identifier"):
				print("Got a port map addition request")
			
			
			#y[1] = 5000 #Android clients use port 5000 to get the data
			addr = tuple(y)
			# new code
			# add only clients port mapped data. 
			self.clients.add(addr)
			self.recvPackets.put((data,addr)) #add the data and sender's data to the queue
			obj.release()

	def run(self): #if there are some received packets broadcast them to all
		threading.Thread(target=self.RecvData).start() #separate thread for listening to the clients

		while True:
			# while not self.recvPackets.empty():
			obj.acquire()
			data,addr = self.recvPackets.get()
			if data:
				types = data[0:4]
				int_val = int.from_bytes(types,"little")
				print(int_val)
				if(int_val/256 == 3):
					self.clients.remove(addr)
					print("Removing", self.clients)
					if(self.clients==set()):
						self.close()
						return
				elif(int_val/256 == 4):
					print("removing all clients")
					for c in self.clients:
						# if c[0] != addr[0]:
						self.s.sendto(data,c)	
						#self.clients.remove(c)
					self.s.close()
					self.clients.clear() #removes all the clients 
					print("Successfully removed all clients")
					return
				for c in self.clients:
					if c[0]==addr[0]:      #Don't send to the actual sender but to all others
						self.s.sendto(data,c)
			#time.sleep(1)
		self.s.close()

	def close(self):
		self.s.close()
