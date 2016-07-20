#!/bin/sh
#
./build_d.sh
rm -f /prod/sloong/sloongnet
rm -rdf /prod/sloong/scripts
mkdir /prod
mkdir /prod/sloong
cp -f output/debug/sloongnet /prod/sloong/sloongnet
cp -rdf output/debug/scripts /prod/sloong/scripts
cp -f output/debug/default.conf /prod/sloong/default.conf
cd ../centos
cp sloongnet /etc/init.d/sloongnet
cp sloongnet.services /usr/lib/systemd/system/sloongnet.services
systemctl daemon-reload