'''
@Author: WCB
@Date: 2019-11-14 15:00:40
@LastEditors: WCB
@LastEditTime: 2019-11-14 16:03:34
@Description: file content
'''
from django.urls import path

from . import views

app_name = 'settings'
urlpatterns = [
    path('',views.index, name='index'),
    path('config/<str:uuid>/', views.config,name='config'),
]