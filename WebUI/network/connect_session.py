'''
@Author: WCB
@Date: 2019-11-14 13:57:27
@LastEditors: WCB
@LastEditTime: 2019-12-12 17:39:17
@Description: Manage connect session.
'''
import socket

class ConnectSession(object):
    '''
    Socket extend module for python.
    User just use send functions, it auto process all other work.
    '''
    _socket = socket.socket()
    _isconnect = False
    _host = 'localhost'
    _port = 8002
    def __init__(self,server,port):
        self._host = server
        self._port = port

    def _connect(self):
        try:
            self._socket.connect((self._host,self._port))
        except Exception as err :
            return (False,err.__str__())
        else:
            self._isconnect = True
            return (True,'')


    def _reconnect_and_sendall( self, data ):
        try:
            self._connect()
            self._socket.sendall(data)
        except socket.error:
            self._reconnect_and_sendall(data)

    '''
    @Remarks: send data to control
    @Params: sendata with bytes
    @Return: recved data
    '''
    def send(self,senddata):
        if not self._isconnect :
            (res,msg) = self._connect()
            if not res:
                return (res,msg)

        len_data = len(senddata).to_bytes(4,byteorder="big")
        try:
            self._socket.sendall(len_data)
        except socket.error:
            self._reconnect_and_sendall(len_data)
        except:
            self._reconnect_and_sendall(len_data)
            
        self._socket.sendall(senddata)
        len_data = self._recv_ex(4)
        data = self._recv_ex(int.from_bytes(len_data,byteorder="big"))
        return (True,data)

    def _recv_ex(self, recv_len):
        data = self._socket.recv(recv_len)
        while len(data) < recv_len :
            data += self._socket.recv(recv_len-len(data))
        return data



