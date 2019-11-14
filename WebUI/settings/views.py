'''
@Author: WCB
@Date: 2019-11-14 14:05:25
@LastEditors: WCB
@LastEditTime: 2019-11-14 17:26:06
@Description: file conten
'''
from django.shortcuts import render
from network import MessagePackage
from protocol import protocol_pb2 as protocol
import json

def index(request):
    context = {}
    context['AppName'] = 'ControlUI'
    pack = MessagePackage.MessagePackage()
    data = pack.Request(protocol.Functions.GetWaitConfigList,'')
    jres = json.loads(data)
    if not jres['WaitConfigList'] :
        context["NoWaitConfigList"] = True
    else:
        context['WaitConfigList'] = jres['WaitConfigList']
    
    return render(request, 'index.html', context)

def config(request,uuid):
    context = {}
    pack = MessagePackage.MessagePackage()
    config_list = {}
    #config_list['ModuleType'] = {tuple(protocol.ModuleType.values())}
    config_list['ListenPort'] = 0
    config_list['EnableSSL'] = False
    config_list['CertFilePath'] = ''
    context['ConfigList'] = config_list
    return render(request, 'config.html', context)