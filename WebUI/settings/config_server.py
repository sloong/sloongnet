'''
@Author: WCB
@Date: 2019-12-12 16:42:59
@LastEditors: WCB
@LastEditTime: 2019-12-14 22:06:09
@Description: file content
'''

from django.shortcuts import render
from django.http import HttpResponseRedirect
from django.http import Http404

from protocol import protocol_pb2 as protocol
from network import message_package
from manage.context import AppContext

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
    def responseGlobal(request, uuid):
        c = __context__.GetContext(request)
        form = None
        # When first post(global config), the COOKIES is no set type.
        # So if no cookies or type not special, view as global.
        if request.COOKIES.get('type') == 'special':
            return requestSpecial(request, uuid)
        form = GlobalConfigForm(request.POST)
        # check the form is not valid
        if not form.is_valid():
            c['form'] = form
            return render(request, 'config.html', c)
        # check module type
        # if is control goto second step
        request.COOKIES['module_type'] = form.module_type
        request.COOKIES['global_config'] = form.to_protocol().SerializeToString()
        if form.module_type == protocol.ModuleType.Control:
            return responseSpecial(request, uuid)
        
        if form.module_type == protocol.ModuleType.Firewall:
            form = FirewallConfigForm()
        elif form.module_type == protocol.ModuleType.Gateway:
            form = GatewayConfigForm()
        elif form.module_type == protocol.ModuleType.Data:
            form = DataConfigForm()
        elif form.module_type == protocol.ModuleType.Process:
            form = ProcessConfigForm()
        elif form.module_type == protocol.ModuleType.DB:
            form = DBConfigForm()
        c['form'] = form
        request.COOKIES['type'] = 'special'
        return render(request, 'config.html', c)

    @staticmethod
    def responseSpecial(request, uuid):
        c = __context__.GetContext(request)
        if request.COOKIES.get('type') == 'special':
            module_type = request.COOKIES.get('module_type')
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
        config.ParseFromString(request.COOKIES.get('global_config'))
        if module_type == protocol.ModuleType.Firewall:
            config.ExConfig = FirewallConfigForm(request.POST).to_protocol().SerializeToString()
        elif module_type == protocol.ModuleType.Gateway:
            config.ExConfig = GatewayConfigForm(request.POST).to_protocol().SerializeToString()
        elif module_type == protocol.ModuleType.Data:
            config.ExConfig = DataConfigForm(request.POST).to_protocol().SerializeToString()
        elif module_type == protocol.ModuleType.Process:
            config.ExConfig = ProcessConfigForm(request.POST).to_protocol().SerializeToString()
        elif module_type == protocol.ModuleType.DB:
            config.ExConfig = DBConfigForm(request.POST).to_protocol().SerializeToString()

        pack = message_package.MessagePackage()
        (res, msg) = pack.Request(protocol.Functions.SetServerConfig, uuid, config.SerializeToString())
        if res == protocol.ResultType.Succeed:
            return HttpResponseRedirect("/settings/")
        else:
            return render(request, 'config.html', {'Response': True, 'Result': False, 'Message': msg})
