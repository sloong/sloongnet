#!/bin/bash
sudo docker run -d --name $1 -p 8001:8001 -v data:/data -e ADDRESS_INFO=$2 sloongnet:0.10.2.412