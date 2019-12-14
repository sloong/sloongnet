'''
@Author: WCB
@Date: 2019-11-15 13:50:33
@LastEditors: WCB
@LastEditTime: 2019-12-14 10:34:45
@Description: file content
'''

from .server import ServerManage
from .models import ServerList

class AppContext():
    _app_name = ''
    context = {}
    def __init__(self,name):
        _app_name = name
        self.context['SiteName'] = 'WebControl'
        self.context['Version'] = '0.0.0.1'
        self.context['ProjectName'] = 'ControlUI'
        self.context['ModuleName'] = _app_name

    def GetContext(self,request):
        serverInfo = ServerManage.get_current(request)
        if serverInfo:
            self.context['CurrentControl'] = serverInfo.name
            self.context['ControlList'] = ServerManage.get_server_list()
        return self.context