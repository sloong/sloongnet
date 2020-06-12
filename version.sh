#!/bin/bash
# build the version text file 
VER_FILE_PATH=src/version.h
rm -f $VER_FILE_PATH
_GIT_VERSION_VALUE=`git rev-list --all | wc -l`
VER_TEXT=$(cat version.h.template | sed "s/GIT_VERSION/$_GIT_VERSION_VALUE/g")
echo "$VER_TEXT" > $VER_FILE_PATH
# find the version template from version.h.template
FILENAME=version.h.template
for  i  in  `cat $FILENAME`
do
	case $i in
		*"GIT_VERSION"*) VER_TEXT=$i;;
	esac
done
VER_TEXT=${VER_TEXT/GIT_VERSION/$_GIT_VERSION_VALUE}
VER_TEXT=${VER_TEXT#*\"}
VER_TEXT=${VER_TEXT%\"*}
# build the version file
VER_PATH=version
rm -f $VER_PATH
echo $VER_TEXT > $VER_PATH
# update version to git
git add $VER_FILE_PATH
git add $VER_PATH