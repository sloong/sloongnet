'''
@Author: WCB
@Date: 2019-12-12 15:50:12
@LastEditors: WCB
@LastEditTime: 2019-12-14 10:28:08
@Description: file content
'''
from django.shortcuts import render
from django.http import HttpResponseRedirect

from manage.server import ServerManage
from manage.context import AppContext

from .forms.addserver_form import AddServerForm

__context__ = AppContext('AddControl')

def add(request):
    if request.method == 'POST':
        return AddControl.response(request)
    else:
        return AddControl.request(request)


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