FROM ubuntu:20.04 AS build-env

RUN apt update && apt install -y ca-certificates

COPY ./build/sources.list /etc/apt/sources.list

RUN cat /etc/apt/sources.list
RUN rm -Rf /var/lib/apt/lists/*

RUN apt update && DEBIAN_FRONTEND="noninteractive" TZ="America/New_York" apt install -y tzdata

RUN apt install -y \
    cmake clang llvm libsqlite3-dev libprotobuf-dev protobuf-compiler uuid-dev libssl-dev libjsoncpp-dev libmariadbclient-dev liblua5.3-dev

COPY . /tmp/
WORKDIR /tmp
# RUN echo $(ls -1 /tmp)
RUN /tmp/build/build.sh -r

#FROM debian:10-slim
FROM ubuntu:20.04
LABEL maintainer="admin@sloong.com"

RUN apt update && apt install -y ca-certificates
COPY ./build/sources.list /etc/apt/sources.list

RUN cat /etc/apt/sources.list
RUN rm -Rf /var/lib/apt/lists/*

RUN apt update && apt install -y \
    libsqlite3-0 libprotobuf17 libuuid1 libssl1.1 libjsoncpp1 libmariadb3 liblua5.3-0 imagemagick
    
WORKDIR /usr/local/bin
COPY --from=build-env /tmp/build/release/ /usr/local/bin/
RUN chmod +x /usr/local/bin/sloongnet

RUN mkdir -p /data/log
VOLUME /data
EXPOSE 8000
ENTRYPOINT ["/usr/local/bin/sloongnet"]
CMD ["Worker","controller:8000"]
