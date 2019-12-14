'''
@Author: WCB
@Date: 2019-12-12 14:38:47
@LastEditors: WCB
@LastEditTime: 2019-12-12 14:39:13
@Description: file content
'''
from django.shortcuts import render
from django.http import HttpResponseRedirect

def home(request):
    return HttpResponseRedirect("/settings/")