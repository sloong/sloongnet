#!/bin/bash
#
INSTALLPATH=/prod/sloongnet
if [ ! -d "$INSTALLPATH" ]; then
	mkdir -p $INSTALLPATH
fi

rm -f $INSTALLPATH/sloongnet
rm -rdf $INSTALLPATH/script
cp -f sloongnet $INSTALLPATH/sloongnet
cp -rf scripts/ $INSTALLPATH/
cp -f default.conf $INSTALLPATH/default.conf
cp centos/sloongnet /etc/init.d/sloongnet
cp centos/sloongnet.service /usr/lib/systemd/system/sloongnet.service
systemctl daemon-reload
