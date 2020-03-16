'''
@Author: WCB
@Date: 2019-12-12 10:25:43
@LastEditors: WCB
@LastEditTime: 2020-03-14 09:30:42
@Description: file content
'''
import uuid as GenUUID

from network.connect_session import ConnectSession
from network.message_package import MessagePackage

from .models import ServerList
from protocol import protocol_pb2 as protocol

# Server manager for all type server.
class ServerManage:
    SessionList = {}
    
    @staticmethod
    def set_current(request,uuid):
        request.COOKIES['current_control']=uuid

    @staticmethod
    def get_current(request):
        id = request.COOKIES.get('current_control')
        serverInfo = None
        try:
            if not id:
                serverInfo = ServerList.objects.all()[0]
            else:
                serverInfo = ServerList.objects.get(uuid=id)
        except IndexError:
            return None
        except ServerList.DoesNotExist:
            return None
        except:
            return None
        return serverInfo
        

    @staticmethod
    def get_server_config(request,uuid):
        pack = MessagePackage()
        cur = ServerManage.get_current_connect(request)
        result = pack.Request(cur, protocol.Functions.GetServerConfig, msg=uuid)
        if result.Result == protocol.ResultType.Succeed:
            return result.Extend
        else:
            return None

    @staticmethod
    def set_server_config(request,uuid,config):
        config_str = config.SerializeToString()
        pack = MessagePackage()
        cur = ServerManage.get_current_connect(request)
        return pack.Request(cur, protocol.Functions.SetServerConfig, msg=uuid, extend=config_str)
        
    
    @staticmethod
    def get_current_connect(request):
        serverInfo = ServerManage.get_current(request)
        if not serverInfo:
            return None
        if not _Instance.SessionList.get(serverInfo.uuid):
            _Instance.SessionList[serverInfo.uuid] = ConnectSession(serverInfo.ip,serverInfo.port)
        return _Instance.SessionList[serverInfo.uuid]

    @staticmethod
    def add_server(address, port):
        serv = ServerList()
        serv.name = 'unknown' 
        serv.notes = 'unknown'
        serv.ip = address
        serv.port = port
        serv.uuid = GenUUID.uuid4()
        serv.tag = 0
        serv.save()
        return serv.uuid.__str__()

    @staticmethod
    def del_server(id):
        ServerList.objects.filter(uuid=id).delete()

    @staticmethod
    def get_server_list():
         return ServerList.objects.all()
        


        
_Instance = ServerManage()