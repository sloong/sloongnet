VERSION_STR=$(cat ../version)
docker build --build-arg BUILD_BRANCH=next_gen -t sloong/sloongnet:$VERSION_STR -f Dockerfile