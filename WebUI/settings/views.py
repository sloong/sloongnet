'''
@Author: WCB
@Date: 2019-11-14 14:05:25
@LastEditors: WCB
@LastEditTime: 2019-11-15 15:30:16
@Description: file conten
'''
from django.shortcuts import render
from django.http import HttpResponseRedirect
from network import MessagePackage
from protocol import protocol_pb2 as protocol
from . import context
import json

def index(request):
    c = context.GetContext()
    pack = MessagePackage.MessagePackage()
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
    config_list['ModuleType'] = tuple(protocol.ModuleType.items()[1:])
    config_list['ListenPort'] = 8002
    config_list['EnableSSL'] = False
    config_list['CertFilePath'] = ''
    config_list['KeyFilePath'] = ''
    config_list['CertPasswd'] = ''
    config_list['ConnectTime'] = 5
    config_list['ReceiveTime'] = 5
    config_list['LogPath']='./log/sloong/'
    config_list['LogLevel']=tuple(protocol.LogLevel.items())
    config_list['DebugMode']=True
    config_list['MQThreadQuantity']=1
    config_list['ProcessThreadQuantity']=1
    config_list['PrioritySize']=5

    c['ConfigList'] = config_list
    return render(request, 'config.html', c)

def config_response(request,uuid):
    pack = MessagePackage.MessagePackage()
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
