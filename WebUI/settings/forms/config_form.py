'''
@Author: WCB
@Date: 2019-12-14 10:43:20
@LastEditors: WCB
@LastEditTime: 2019-12-14 22:01:20
@Description: file content
'''
from django import forms
from protocol import protocol_pb2 as protocol


class GlobalConfigForm(forms.Form):
    #module_type = forms.ChoiceField(choices=protocol.ModuleType.items()[2:],initial=2,widget=forms.Select(attrs={'onchange':'OnModuleTypeChange();'}))
    module_type = forms.ChoiceField(choices=protocol.ModuleType.items()[2:],initial=2)
    listen_port = forms.IntegerField( max_value=65535, min_value=1000)
    enable_ssl = forms.BooleanField()
    cert_file_path = forms.CharField(required=False)
    key_file_path = forms.CharField(required=False)
    cert_passwd = forms.CharField(required=False)
    connect_time = forms.IntegerField(min_value=0,max_value=20,help_text="set 0 to disabled")
    receive_time = forms.IntegerField(min_value=0, max_value=20, help_text='set 0 to disabled')
    log_path = forms.CharField()
    log_level = forms.ChoiceField(choices=protocol.LogLevel.items())
    debug_mode = forms.BooleanField()
    MQ_thread_quantity = forms.IntegerField(min_value=1,max_value=200)
    process_thread_quantity = forms.IntegerField(min_value=1,max_value=200)
    priority_size = forms.IntegerField(min_value=0,max_value=10)
    def to_protocol(self):
        config = protocol.GLOBAL_CONFIG()
        config.Type = int(self.cleaned_data['module_type'])
        config.ListenPort = int(self.cleaned_data['listen_port'])
        config.EnableSSL = bool(self.cleaned_data['enable_ssl'])
        config.CertFilePath = self.cleaned_data['cert_file_path']
        config.KeyFilePath = self.cleaned_data['key_file_path']
        config.CertPasswd = self.cleaned_data['cert_passwd']
        config.ConnectTime = int(self.cleaned_data['connect_time'])
        config.ReceiveTime = int(self.cleaned_data['receive_time'])
        config.LogPath = self.cleaned_data['log_path']
        config.LogLevel = int(self.cleaned_data['log_level'])
        config.DebugMode = bool(self.cleaned_data['debug_mode'])
        config.MQThreadQuantity = int(self.cleaned_data['MQ_thread_quantity'])
        config.ProcessThreadQuantity = int(self.cleaned_data['process_thread_quantity'])
        config.PrioritySize = int(self.cleaned_data['priority_size'])
        return config

class FirewallConfigForm(forms.Form):
    block_time = forms.IntegerField(min_value=1, max_value=100)
    def to_protocol(self):
        config = protocol.FIREWALL_CONFIG()
        config.BlockTime = int(self.cleaned_data['block_time'])
        return config


class ProcessConfigForm(forms.Form):
    lua_context_quantity = forms.IntegerField(min_value=1,  max_value=300)
    lua_script_folder = forms.CharField()
    lua_entry_file = forms.CharField(initial='main.lua')
    lua_entry_function = forms.CharField(initial='init')
    lua_process_function = forms.CharField(initial='process')
    lua_socket_close_function = forms.CharField(initial='socketclose')
    def to_protocol(self):
        config = protocol.PROCESS_CONFIG()
        config.LuaContextQuantity = int(self.cleaned_data['lua_context_quantity'])
        config.LuaScriptFolder = self.cleaned_data['lua_script_folder']
        config.LuaEntryFile = self.cleaned_data['lua_entry_file']
        config.LuaEntryFunction = self.cleaned_data['lua_entry_function']
        config.LuaProcessFunction = self.cleaned_data['lua_process_function']
        config.LuaSocketCloseFunction = self.cleaned_data['lua_socket_close_function']
        return config

class GatewayConfigForm(forms.Form):
    client_check_time = forms.IntegerField()
    client_check_key = forms.CharField(max_length=65,required=False)
    timeout_check_interval = forms.IntegerField()
    timeout_value = forms.IntegerField()
    def to_protocol(self):
        config = protocol.GATEWAY_CONFIG()
        config.ClientCheckTime = int(self.changed_data['client_check_time'])
        config.ClientCheckKey = self.changed_data['client_check_key']
        config.TimeoutCheckInterval = int(self.changed_data['timeout_check_interval'])
        config.TimeoutTime = int(self.changed_data['timeout_value'])
        return config

class DataConfigForm(forms.Form):
    data_receive_port = forms.IntegerField()
    data_receive_time = forms.IntegerField()
    def to_protocol(self):
        config = protocol.DATA_CONFIG()
        config.DataReceivePort = int(self.changed_data['data_receive_port'])
        config.DataReceiveTime = int(self.changed_data['data_receive_time'])
        return config


class DBConfigForm(forms.Form):
    db_server_address = forms.CharField()
    db_server_port = forms.CharField()
    db_user = forms.CharField()
    db_password = forms.PasswordInput()
    db_database = forms.CharField()
    def to_protocol(self):
        config = protocol.DB_CONFIG()
        config.ServerAddress = self.changed_data['db_server_address']
        config.ServerPort = self.changed_data['db_server_port']
        config.User = self.changed_data['db_user']
        config.Passwd = self.changed_data['db_password']
        config.Database = self.changed_data['db_database']
        return config