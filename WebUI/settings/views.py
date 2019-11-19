'''
@Author: WCB
@Date: 2019-11-14 14:05:25
@LastEditors: WCB
@LastEditTime: 2019-11-19 16:16:41
@Description: file conten
'''
from django.shortcuts import render
from django.http import HttpResponseRedirect
from network import message_package
from protocol import protocol_pb2 as protocol
from . import context
import json

def index(request):
    c = context.GetContext()
    pack = message_package.MessagePackage()
    (code,data) = pack.Request(protocol.Functions.GetWaitConfigList,'')
    if code != protocol.ResultType.Succeed:
        c['ErrorMessage'] = data
    jres = json.loads(data)
    if not jres['WaitConfigList'] :
        c["NoWaitConfigList"] = True
    else:
        c['WaitConfigList'] = jres['WaitConfigList']
    
    return render(request, 'index.html', c)

def config(request,uuid):
    if not request.GET:
        return config_show(request,uuid)
    else:
        return config_response(request,uuid)


def config_show(request,uuid):
    c = context.GetContext()
    config_list = {}
    '''
    这里的结构为多级列表，每一条为一个选项。
    每个选项需要多个参数，为一个tuple.
    1. 类型 为str类型文本.
    2. 默认值
    3. 是否必填
    4. 规则描述。int为tuple(min,max)。string为正则表达式。enum为enum列表。None为不启用
    '''
    config_list['ModuleType'] =  ( 'tuple', 1, True, (protocol.ModuleType.items()[2:]))
    config_list['ListenPort'] = ( 'int', 8002, True, ( 1000,65535 ))
    config_list['EnableSSL'] =  ( 'bool', False, True, None )
    config_list['CertFilePath'] = ( 'str', '', False, None) 
    config_list['KeyFilePath'] = ( 'str','', False, None) 
    config_list['CertPasswd'] = ( 'str','', False, None) 
    config_list['ConnectTime'] = ( 'int',5, True, (0, 20))
    config_list['ReceiveTime'] = ( 'int',5, True, (0, 20))
    config_list['LogPath'] = ( 'str','/data/log/', True, None)
    config_list['LogLevel']= ( 'tuple',protocol.LogLevel.Info, True, (protocol.LogLevel.items())) 
    config_list['DebugMode']= ( 'bool',False, True, None )
    config_list['MQThreadQuantity']=  ( 'int',1, True, (1, 20))
    config_list['ProcessThreadQuantity']=  ( 'int',1, True, (1, 20))
    config_list['PrioritySize']=  ( 'int',3, True, (0, 20))
    c['ConfigList'] = config_list
    c['Message']=''
    return render(request, 'config.html', c)

def config_response(request,uuid):
    pack = message_package.MessagePackage()
    config = protocol.GLOBAL_CONFIG()
    config.Type = int(request.GET['ModuleType'])
    config.ListenPort = int(request.GET['ListenPort'])
    config.EnableSSL = bool(request.GET.get('EnableSSL','False'))
    config.CertFilePath = request.GET['CertFilePath']
    config.KeyFilePath = request.GET['KeyFilePath']
    config.CertPasswd = request.GET['CertPasswd']
    config.ConnectTime = int(request.GET['ConnectTime'])
    config.ReceiveTime = int(request.GET['ReceiveTime'])
    config.LogPath = request.GET['LogPath']
    config.LogLevel = int(request.GET['LogLevel'])
    config.DebugMode = bool(request.GET.get('DebugMode','Flase'))
    config.MQThreadQuantity = int(request.GET['MQThreadQuantity'])
    config.ProcessThreadQuantity = int(request.GET['ProcessThreadQuantity'])
    config.PrioritySize = int(request.GET['PrioritySize'])

    (res,msg) = pack.Request(protocol.Functions.SetServerConfig, uuid, config.SerializeToString())
    if res == protocol.ResultType.Succeed:
        return HttpResponseRedirect("/settings/")
    else:
        return render(request, 'config.html', {'Response':True,'Result':False,'Message':msg})

