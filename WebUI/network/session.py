import socket

from protocol.protocol_pb2 import *
#from protocol.protocol import *


class MessagePackage:
    s = socket.socket()
    isconnect = False
    host = 'localhost'
    port = 8002
    def __init__(self):
        self.host = '10.0.0.18'
        self.port = 8002

    def connect(self):
        self.s.connect((self.host,self.port))
        self.isconnect = True

    def send(self):
        if not self.isconnect :
            self.connect()
        send_pack = DataPackage()
        send_pack.Function = Functions.GetWaitConfigList
        senddata = send_pack.SerializeToString()
        self.s.sendall(len(senddata).to_bytes(4,byteorder="big"))
        self.s.sendall(senddata)
        len_data = self.s.recv(4)
        while  len(len_data) < 4 :
            len_data += self.s.recv(4-len(len_data))
        data = self.s.recv(int.from_bytes(len_data,byteorder="big"))
        recv_pack = DataPackage()
        recv_pack.ParseFromString(data)
        return recv_pack.Content


pack = MessagePackage()

def GetMessagePackage():
    return pack