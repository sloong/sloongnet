#!/bin/bash
rm -f sloongnet/version.h
git rev-list HEAD | sort > config.git-hash
GIT_VERSION=`git rev-list --all | wc -l`

cat version.h.template | sed "s/\$FULL_VERSION/$GIT_VERSION/g" > sloongnet/version.h
 
echo "Generated version.h"