from django import forms
from protocol import protocol_pb2 as protocol


class ConfigForm(forms.Form):
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
