
cp sloong.conf /etc/ld.so.conf.d/sloong.conf
mkdir -p /usr/local/include/univ
mkdir -p /usr/local/lib/sloong
cp -f include/univ/*.h /usr/local/include/univ/
cp -f libuniv.so /usr/local/lib/sloong/
