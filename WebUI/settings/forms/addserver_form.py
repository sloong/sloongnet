'''
@Author: WCB
@Date: 2019-12-12 15:41:58
@LastEditors: WCB
@LastEditTime: 2019-12-12 17:22:21
@Description: file content
'''
from django import forms


class AddServerForm(forms.Form):
    Address = forms.CharField(required=False)
    Port = forms.IntegerField( max_value=65535, min_value=1000)
    
