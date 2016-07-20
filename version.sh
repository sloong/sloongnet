#!/bin/bash
VER_PATH=sloongnet/version.h
rm -f $VER_PATH
_GIT_VERSION=`git rev-list --all | wc -l`
cat version.h.template | sed "s/\$GIT_VERSION/$_GIT_VERSION/g" > $VER_PATH
 echo "Generated version.h"
git add $VER_PATH