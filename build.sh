#!/bin/bash
#
show_help(){
	echo -e "param error! \n-r: to build release version \n-d: to build debug version \n-rz: build release to tar.gz\n-dz: build debug to tar.gz"
}

# default value is release
VERSION_STR=$(cat version)
OUTPATH=sloongnet_v$VERSION_STR
TARGPATH=sloongnet
MAKEFLAG=release

build(){
	cd $TARGPATH
	make clean
	make $MAKEFLAG
	cd ..
	rm -rdf $OUTPATH
	mkdir -p $OUTPATH
	cp -f $TARGPATH/sloongnet $OUTPATH/sloongnet
	cp -rf scripts $OUTPATH/scripts
	cp -f default.conf $OUTPATH/default.conf
	cp -f install.sh $OUTPATH/install.sh
}

build_debug(){
	OUTPATH=sloongnet_debug_v$VERSION_STR
	MAKEFLAG=debug
	build
}

zip(){
	tar -czf sloongnet_v$VERSION_STR.tar.gz $OUTPATH/*
}


if [ $# -lt 1 ]
then
	show_help
else
	case $1 in 
		-r) build;;
		-d) build_debug;;
		-rz) 
			build
			zip;;
		-dz) 
			build_debug
			zip;;
		* ) show_help;;
	esac
	
fi
