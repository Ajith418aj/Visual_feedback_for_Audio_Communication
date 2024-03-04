from socket import socket as Socket
from select import select
from json import loads, dumps

def recv(socket: Socket, decode=False):
    '''
    Receives a python object in stream
    decode: decodes using json
    returns None if connection is closed
    '''
    length = b''
    #print('recv called ' )
     
    while len(length) < 4: #first 4 bytes should be the length
        chunk = socket.recv(4 - len(length))
        if chunk == b'':
            # print("here")
            return None
        length += chunk
    length = int.from_bytes(length, 'big')
    received = 0
    chunks = []
    while received < length: #Now recieve the actual data
        chunk = socket.recv(length - received)
        if chunk == b'':
            return None
        chunks.append(chunk)
        received += len(chunk)
    data = b''.join(chunks)
    if decode:
        data = loads(data)
    return data


def send(socket: Socket, data, encode=False):
    '''
    Sends a python object in stream.
    encode: encodes using json
    '''
    if encode:
        data = dumps(data).encode('utf-8')
    data = len(data).to_bytes(4, 'big') + data
    # print("sending")
    socket.sendall(data)
