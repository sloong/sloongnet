'''
@Author: WCB
@Date: 2019-12-12 15:04:25
@LastEditors: WCB
@LastEditTime: 2019-12-12 15:04:26
@Description: file content
'''

# Create your views here.
from django.http import HttpResponse

# Create your views here.
def index(request):
    return HttpResponse("Hello, world. You're at the polls index.")