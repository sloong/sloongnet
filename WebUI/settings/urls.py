'''
@Author: WCB
@Date: 2019-11-14 15:00:40
@LastEditors: WCB
@LastEditTime: 2019-12-12 22:43:42
@Description: file content
'''
from django.urls import path
from . import views
from . import controler
from . import config_server

app_name = 'settings'
urlpatterns = [
    path('',views.index, name='index'),
    path('set_current_control/<str:uuid>/', views.set_current_control, name='set_current_control'),
    path('config/<str:uuid>/', config_server.index,name='config_server'),
    path('list_control', controler.get_list, name='list_control'),
    path('add_control', controler.add,name='add_control'),
    path('del_control/<str:uuid>/', controler.delete,name='del_control'),
]