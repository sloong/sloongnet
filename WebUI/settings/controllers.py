'''
@Author: WCB
@Date: 2019-12-12 15:50:12
@LastEditors: WCB
@LastEditTime: 2020-03-14 09:32:39
@Description: manager for control modules.
'''
from django.shortcuts import render
from django.http import HttpResponseRedirect

from manage.server import ServerManage
from manage.context import AppContext
from network.message_package import MessagePackage

from .forms.addserver_form import AddServerForm
from .forms.config_form import *

__context__ = AppContext('AddControl')

def add(request):
    if request.method == 'POST':
        return AddControl.response(request)
    else:
        return AddControl.request(request)

def config(request,uuid):
    if request.method == 'POST':
        return ConfigControl.response(request,uuid)
    else:
        return ConfigControl.request(request,uuid)

def delete(request,uuid):
    ServerManage.del_server(uuid)
    return HttpResponseRedirect('/settings/list_control')

def get_list(request):
    c = __context__.GetContext(request)
    c['ControlList'] = ServerManage.get_server_list()
    return render(request, 'listcontrol.html', c)

class AddControl:
    @staticmethod
    def request(req):
        c = __context__.GetContext(req)
        c['form'] = AddServerForm()
        return render(req, 'addcontrol.html', c)

    @staticmethod
    def response(req):
        form = AddServerForm(req.POST)
        c = __context__.GetContext(req)

        if form.is_valid():
            uuid = ServerManage.add_server(form.cleaned_data['Address'],form.cleaned_data['Port'])
            return HttpResponseRedirect('/settings/set_current_control/'+uuid+'/')
        else:
            c['form'] = form
            return render(req, 'addcontrol.html', c)

class ConfigControl:
    @staticmethod
    def request(request,uuid):
        c = __context__.GetContext(request)
        config_str = ServerManage.get_server_config(request,uuid)
        form = ControlForm()
        if config_str :
            config = protocol.GLOBAL_CONFIG()
            config.ParseFromString(config_str)
            form = FormConvert.protocol_to_form(config)
        c['form'] = form
        c['Message']=''
        return render(request, 'config.html', c)

    @staticmethod
    def response(request,uuid):
        c = __context__.GetContext(request)
        form = ControlForm(request.POST)

        if not form.is_valid():
            c['form'] = form
            return render(request, 'config.html', c)

        config = FormConvert.to_protocol(form)
        res = ServerManage.set_server_config(request,uuid,config)
        
        if res.Result == protocol.ResultType.Succeed:
            return HttpResponseRedirect("/settings/")
        else:
            return render(request, 'config.html', {'Response': True, 'Result': False, 'Message': res.Context})


