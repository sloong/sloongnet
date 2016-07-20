#!/bin/sh
#
cd sloongnet
make clean
make release
cd ..
rm -rdf output
mkdir output
cp sloongnet/sloongnet output/sloongnet
cp -rdf sloongnet/scripts output/scripts
cp sloongnet/default.conf output/default.conf
