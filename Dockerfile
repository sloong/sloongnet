FROM gcc:9.3 AS build-env

RUN apt update && apt install -y \
    unzip cmake libsqlite3-dev libprotobuf-dev protobuf-compiler uuid-dev libssl-dev libjsoncpp-dev libmariadbclient-dev libluajit-5.1-dev

COPY . /tmp/
WORKDIR /tmp
# RUN echo $(ls -1 /tmp)
RUN /tmp/build/build.sh -r

#FROM debian:10-slim
FROM ubuntu:20.04

LABEL maintainer="admin@sloong.com"
ENV ADDRESS_INFO=0.0.0.0:8000
ENV NODE_TYPE=Worker

RUN apt update && apt install -y \
    libsqlite3-0 libprotobuf17 libuuid1 libssl1.1  libjsoncpp1 libmariadb3 libluajit-5.1-2
WORKDIR /usr/local/bin
COPY --from=build-env /tmp/build/sloongnet-release /usr/local/bin
RUN chmod +x /usr/local/bin/sloongnet

RUN mkdir -p /data/log
VOLUME /data
EXPOSE 8000
ENTRYPOINT ["/usr/local/bin/sloongnet"]
CMD [$NODE_TYPE,$ADDRESS_INFO]