'''
@Author: WCB
@Date: 2019-11-14 13:59:50
@LastEditors: WCB
@LastEditTime: 2020-03-13 18:43:10
@Description: Message package module
'''
from protocol import protocol_pb2 as protocol
from . import connect_session

class MessagePackage:
    pack = protocol.DataPackage()
    def __init(self):
        self.pack.Sender = "0"

    def Request(self, session, func, msg, extend= b'' ):
        self.pack.Function = func
        self.pack.Content = msg
        self.pack.Extend = extend
        (res,recv_data) = session.send(self.pack.SerializeToString())

        if not res:
            return (protocol.ResultType.Error, recv_data)
        recv_pack = protocol.DataPackage()

        recv_pack.ParseFromString(recv_data)
        return recv_pack
