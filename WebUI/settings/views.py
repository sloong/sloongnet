from django.shortcuts import render
from network import session
import network

# Create your views here.
from django.http import HttpResponse

def index(request):
    pack = session.GetMessagePackage()
    data = pack.send()
    return HttpResponse(data)