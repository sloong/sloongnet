#!/bin/bash
#

clean(){
	rm -rdf debug
	mkdir debug
}

build(){
	cd debug
	cmake -DCMAKE_BUILD_TYPE=Debug ../../sloongnet
	make
}
clean_build(){
	clean
	build
}

if [ $# -lt 1 ]
then
	build
else
	clean_build
fi
