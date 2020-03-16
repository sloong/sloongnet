'''
@Author: WCB
@Date: 2019-11-14 15:00:40
@LastEditors: WCB
@LastEditTime: 2020-03-13 11:12:00
@Description: file content
'''
from django.urls import path
from . import views
from . import controllers
from . import template_manage
from . import module_manage

app_name = 'settings'
urlpatterns = [
    path('',views.index, name='index'),
    path('set_current_control/<str:uuid>/', views.set_current_control, name='set_current_control'),
    path('list_templates',template_manage.list, name='list_templates'),
    path('list_modules',module_manage.list, name='list_modules'),
    path('config/<str:uuid>/', module_manage.config,name='config_server'),
    path('config_control/<str:uuid>/', controllers.config, name='config_control'),
    path('config_template/<str:id>/', template_manage.config, name='config_template'),
    path('list_control', controllers.get_list, name='list_control'),
    path('add_control', controllers.add,name='add_control'),
    path('del_control/<str:uuid>/', controllers.delete,name='del_control'),
]