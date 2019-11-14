'''
@Author: WCB
@Date: 2019-11-14 13:57:27
@LastEditors: WCB
@LastEditTime: 2019-11-14 14:04:17
@Description: Manage connect session.
'''
import socket

class ConnectSession:
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

    '''
    @Remarks: send data to control
    @Params: sendata with bytes
    @Return: recved data
    '''
    def send(self,senddata):
        if not self.isconnect :
            self.connect()

        self.s.sendall(len(senddata).to_bytes(4,byteorder="big"))
        self.s.sendall(senddata)
        len_data = self.s.recv(4)
        while  len(len_data) < 4 :
            len_data += self.s.recv(4-len(len_data))
        data = self.s.recv(int.from_bytes(len_data,byteorder="big"))
        return data


session = ConnectSession()
