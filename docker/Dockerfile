FROM ubuntu:18.04

RUN apt-get update

RUN apt-get install -y libsqlite3-0 libprotobuf10 libuuid1 libglib2.0-0 libssl1.1 libboost-serialization1.65.1 liblua5.3-0 libjsoncpp1 wget unzip

RUN apt-get install -y cmake gcc g++ gdb libsqlite3-dev libprotobuf-dev protobuf-compiler uuid-dev libglib2.0-dev libssl-dev libboost-dev libboost-serialization-dev liblua5.3-dev libjsoncpp-dev

ARG BUILD_BRANCH=master
ARG LIBUNIV_VER=2.4.5.227

RUN wget https://github.com/sloong/sloongnet/archive/${BUILD_BRANCH}.zip \
    && unzip ${BUILD_BRANCH}.zip \
    && apk del --purge wget \
	&& rm ${BUILD_BRANCH}.zip

WORKDIR ${BUILD_BRANCH}

RUN mkdir /app && mkdir /data

RUN tar -xvzf ./reference/libuniv_v${LIBUNIV_VER}.tar.gz \
    && cp ./reference/libuniv_release/${LIBUNIV_VER}/libuniv.so /app/

RUN bash ./build/build.sh -r \
    && cp ./build/release/sloongnet /app/ \
    && cp ./sloongnet/script/ /app/ \
    && cp ./*.so /app/ 

VOLUME /data

ENV ADDRESS_INFO=127.0.0.1:8001

EXPOSE 8001
WORKDIR  /app
ENTRYPOINT ["/app/sloongnet", ${ADDRESS_INFO}]

