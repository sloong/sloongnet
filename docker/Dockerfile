FROM ubuntu:18.04

LABEL maintainer="admin@sloong.com"

RUN apt-get update \
    && apt-get install -y libsqlite3-0 libprotobuf10 libuuid1 libglib2.0-0 libssl1.1 libboost-serialization1.65.1 liblua5.3-0 libjsoncpp1

RUN mkdir /app && mkdir /data

WORKDIR  /app

COPY ./output/ /app/
COPY run.sh /app/

RUN chmod 755 sloongnet

VOLUME /data

ENV ADDRESS_INFO=127.0.0.1:8001

EXPOSE 8001

CMD ["/app/run.sh"]

