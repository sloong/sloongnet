'''
@Author: WCB
@Date: 2019-11-14 14:05:25
@LastEditors: WCB
@LastEditTime: 2020-03-13 11:22:03
@Description: file conten
'''
from django.shortcuts import render
from django.http import HttpResponseRedirect
from django.http import Http404

from protocol import protocol_pb2 as protocol

from network.message_package import MessagePackage
from network.connect_session import ConnectSession

from manage import models
from manage.context import AppContext
from manage.server import ServerManage

from .forms.config_form import *

import json

__context__ = AppContext('Setting')

def list(request):
    c = __context__.GetContext(request)
    cur = ServerManage.get_current_connect(request)
    if not cur:
        return HttpResponseRedirect('/settings/add_control')
    else:
        pack = MessagePackage()
        (code,data) =pack.Request(cur, protocol.Functions.GetAllConfigTemplate,'')
        if code != protocol.ResultType.Succeed:
            c['ErrorMessage'] = data
        else:
            jres = json.loads(data)
            c['ConfigTemplateList'] = jres['ConfigTemplateList']
    return render(request, 'list_templates.html', c)



def config(request,uuid):
    if request.method == 'POST':
        return config_response(request,uuid)
    else:
        return config_request(request,uuid)

# 这里需要把获取到的模板列表的类型保存下来
def getCurrentServerType():
    return protocol.ModuleType.Firewall

def config_request(request,uuid):
    c = __context__.GetContext(request)
    module_type = getCurrentServerType()
    if module_type == protocol.ModuleType.Control:
        form = ControlForm()
    elif module_type == protocol.ModuleType.Firewall:
        form = FirewallConfigForm()
    elif module_type == protocol.ModuleType.Gateway:
        form = GatewayConfigForm()
    elif module_type == protocol.ModuleType.Data:
        form = DataConfigForm()
    elif module_type == protocol.ModuleType.Process:
        form = ProcessConfigForm()
    elif module_type == protocol.ModuleType.DB:
        form = DBConfigForm()
    else:
        return Http404
    c['form'] = form
    c['Message']=''
    return render(request, 'config.html', c)

def config_response(request,uuid):
    c = __context__.GetContext(request)
    module_type = getCurrentServerType()
    if module_type == protocol.ModuleType.Control:
        form = ControlForm(request.POST)
    elif module_type == protocol.ModuleType.Firewall:
        form = FirewallConfigForm(request.POST)
    elif module_type == protocol.ModuleType.Gateway:
        form = GatewayConfigForm(request.POST)
    elif module_type == protocol.ModuleType.Data:
        form = DataConfigForm(request.POST)
    elif module_type == protocol.ModuleType.Process:
        form = ProcessConfigForm(request.POST)
    elif module_type == protocol.ModuleType.DB:
        form = DBConfigForm(request.POST)
    else:
        return Http404

    if not form.is_valid():
        c['form'] = form
        return render(request, 'config.html', c)

    config_str = FormConvert.to_protocol(form).SerializeToString()

    pack = MessagePackage()
    cur = ServerManage.get_current_connect(request)
    (res, msg) = pack.Request(cur, protocol.Functions.SetServerConfig, msg=uuid, extend=config_str)
    if res == protocol.ResultType.Succeed:
        return HttpResponseRedirect("/settings/")
    else:
        return render(request, 'config.html', {'Response': True, 'Result': False, 'Message': msg})


