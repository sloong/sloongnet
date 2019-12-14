'''
@Author: WCB
@Date: 2019-11-14 14:05:25
@LastEditors: WCB
@LastEditTime: 2019-12-12 22:27:50
@Description: file conten
'''
from django.shortcuts import render
from django.http import HttpResponseRedirect

from protocol import protocol_pb2 as protocol

from network.message_package import MessagePackage
from network.connect_session import ConnectSession

from manage.context import AppContext


from manage import models
from manage.server import ServerManage
import json

__context__ = AppContext('Setting')

def index(request):
    c = __context__.GetContext(request)
    cur = ServerManage.get_current_connect(request)
    if not cur:
        return HttpResponseRedirect('/settings/add_control')
    else:
        pack = MessagePackage()
        (code,data) =pack.Request(cur, protocol.Functions.GetWaitConfigList,'')
        if code != protocol.ResultType.Succeed:
            c['ErrorMessage'] = data
        else:
            jres = json.loads(data)
            if not jres['WaitConfigList'] :
                c["NoWaitConfigList"] = True
            else:
                c['WaitConfigList'] = jres['WaitConfigList']
    
    return render(request, 'index.html', c)


def set_current_control(request, uuid):
    ServerManage.set_current(request,uuid)
    return HttpResponseRedirect('/settings')