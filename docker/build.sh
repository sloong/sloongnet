#!/bin/bash
sudo pwd
SCRIPTFOLDER=$(dirname $(readlink -f $0))
echo "ScriptFolder: "$SCRIPTFOLDER
# cd to current file folder
cd $SCRIPTFOLDER

../build/build.sh -rz
VERSION_STR=$(cat ../version)

PROJECT=sloongnet

mv ../$PROJECT-v$VERSION_STR.zip output.zip
unzip -o output.zip -d output

sudo docker build -t sloong/sloongnet:$VERSION_STR .