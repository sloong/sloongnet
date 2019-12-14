'''
@Author: WCB
@Date: 2019-11-14 15:00:40
@LastEditors: WCB
@LastEditTime: 2019-12-12 15:03:22
@Description: file content
'''
from django.urls import path
from . import views

app_name = 'control'
urlpatterns = [
    path('',views.index, name='index'),
]