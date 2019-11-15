'''
@Author: WCB
@Date: 2019-11-14 16:42:53
@LastEditors: WCB
@LastEditTime: 2019-11-15 11:49:41
@Description: file content
'''
from django import template

register = template.Library()

@register.filter(name='gettype')
def gettype(value):
    return type(value).__name__
    
@register.filter(name='isint')
def isint(value):
    return isinstance(value,int)

@register.filter(name='isbool')
def isbool(value):
    return isinstance(value,bool)


@register.filter(name='isstr')
def isstring(value):
    return isinstance(value,str)

@register.filter(name='istuple')
def istuple(value):
    return isinstance(value,tuple)