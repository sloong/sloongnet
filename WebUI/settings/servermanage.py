'''
@Author: WCB
@Date: 2019-11-14 14:05:25
@LastEditors: WCB
@LastEditTime: 2019-12-12 10:19:34
@Description: file conten
'''
from django.shortcuts import render
from django.http import HttpResponseRedirect
from network import message_package
from protocol import protocol_pb2 as protocol
from . import context
from .forms.config_form import GlobalConfigForm
import json

def list_server(request):
    
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
    if request.method == 'POST':
        return config_response(request,uuid)
    else:
        return config_show(request,uuid)


def config_show(request,uuid):
    c = context.GetContext()
    c['form'] = GlobalConfigForm()
    
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

