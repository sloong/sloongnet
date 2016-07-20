#!/bin/bash
#
cd sloongnet
make clean
make debug
cd ..
rm -rdf output
mkdir output
cp sloongnet/sloongnet output/debug/sloongnet
cp -rdf sloongnet/scripts output/debug/scripts
cp sloongnet/default.conf output/debug/default.conf
