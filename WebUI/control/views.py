'''
@Author: WCB
@Date: 2019-12-12 15:02:47
@LastEditors: WCB
@LastEditTime: 2019-12-12 15:04:05
@Description: file content
'''
from django.http import HttpResponse

# Create your views here.
def index(request):
    return HttpResponse("Hello, world. You're at the polls index.")