cd sloongnet
make clean
rm -f /prod/sloong/sloongnet
make release
mkdir /prod
mkdir /prod/sloong
cp -f sloongnet /prod/sloong/sloongnet
cp -rdf scripts /prod/sloong/scripts
cp -f default.conf /prod/sloong/default.conf
cd ../centos
cp sloongnet /etc/init.d/sloongnet
cp sloongnet.services /usr/lib/systemd/system/sloongnet.services
systemctl daemon-reload