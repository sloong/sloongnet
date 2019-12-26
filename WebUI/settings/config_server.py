'''
@Author: WCB
@Date: 2019-12-12 16:42:59
@LastEditors  : WCB
@LastEditTime : 2019-12-24 16:21:35
@Description: file content
'''

from django.shortcuts import render
from django.http import HttpResponseRedirect
from django.http import Http404

from protocol import protocol_pb2 as protocol
from network import message_package
from manage.context import AppContext
from manage.server import ServerManage

from .forms.config_form import *

__context__ = AppContext('ConfigServer')


def index(request, uuid):
    if request.method == 'POST':
        return ConfigServer.response(request, uuid)
    else:
        return ConfigServer.request(request, uuid)


class ConfigServer:
    @staticmethod
    def request(req, uuid):
        c = __context__.GetContext(req)
        c['form'] = GlobalConfigForm()
        c['type'] = 'global'
        return render(req, 'config.html', c)

    @staticmethod
    def responseSpecial(request, uuid):
        c = __context__.GetContext(request)
        if request.COOKIES.get('type') == 'special':
            module_type = int(request.COOKIES.get('module_type'))
            if module_type == protocol.ModuleType.Control:
                form = GlobalConfigForm(request.POST)
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
        else:
            return Http404()

        if not form.is_valid():
            c['form'] = form
            return render(request, 'config.html', c)

        config = protocol.GLOBAL_CONFIG()
        config.ParseFromString(bytes.fromhex(request.COOKIES.get('global_config')))
        config.ExConfig = FormConvert.to_protocol(form).SerializeToString()

        pack = message_package.MessagePackage()
        cur = ServerManage.get_current_connect(request)
        (res, msg) = pack.Request(cur,protocol.Functions.SetServerConfig, uuid, config.SerializeToString())
        if res == protocol.ResultType.Succeed:
            return HttpResponseRedirect("/settings/")
        else:
            return render(request, 'config.html', {'Response': True, 'Result': False, 'Message': msg})

    @staticmethod
    def response(request, uuid):
        c = __context__.GetContext(request)
        form = None
        # When first post(global config), the COOKIES is no set type.
        # So if no cookies or type not special, view as global.
        if request.COOKIES.get('type') == 'special':
            return ConfigServer.responseSpecial(request, uuid)
        form = GlobalConfigForm(request.POST)
        # check the form is not valid
        if not form.is_valid():
            c['form'] = form
            return render(request, 'config.html', c)
        # check module type
        # if is control goto second step
        module_type = int(form.cleaned_data['module_type'])
        global_config_str = FormConvert.to_protocol(form).SerializeToString().hex()
        if module_type == protocol.ModuleType.Control:
            return ConfigServer.responseSpecial(request, uuid)
        
        if module_type == protocol.ModuleType.Firewall:
            form = FirewallConfigForm()
        elif module_type == protocol.ModuleType.Gateway:
            form = GatewayConfigForm()
        elif module_type == protocol.ModuleType.Data:
            form = DataConfigForm()
        elif module_type == protocol.ModuleType.Process:
            form = ProcessConfigForm()
        elif module_type == protocol.ModuleType.DB:
            form = DBConfigForm()
        c['form'] = form
        response=render(request, 'config.html', c)
        response.set_cookie('module_type',module_type)
        response.set_cookie('type','special')
        response.set_cookie('global_config',global_config_str)
        return response

    