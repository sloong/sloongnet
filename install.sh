#!/bin/bash
#
./build.sh
rm -f /prod/sloong/sloongnet
rm -rdf /prod/sloong/scripts
mkdir /prod
mkdir /prod/sloong
cp -f output/sloongnet /prod/sloong/sloongnet
cp -rdf output/scripts /prod/sloong/scripts
cp -f output/default.conf /prod/sloong/default.conf
cd ../centos
cp sloongnet /etc/init.d/sloongnet
cp sloongnet.services /usr/lib/systemd/system/sloongnet.services
systemctl daemon-reload