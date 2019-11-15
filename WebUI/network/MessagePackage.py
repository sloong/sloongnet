'''
@Author: WCB
@Date: 2019-11-14 13:59:50
@LastEditors: WCB
@LastEditTime: 2019-11-15 15:07:33
@Description: Message package module
'''
from protocol import protocol_pb2 as protocol
from . import ConnectSession

class MessagePackage:
    pack = protocol.DataPackage()
    def __init(self):
        self.pack.Sender = "0"

    def Request(self, func, msg ):
        self.pack.Function = func
        self.pack.Content = msg
        recv_data = ConnectSession.session.send(self.pack.SerializeToString())
        recv_pack = protocol.DataPackage()

        recv_pack.ParseFromString(recv_data)
        return (recv_pack.Result,recv_pack.Content)


