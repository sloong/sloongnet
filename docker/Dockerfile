FROM ubuntu:18.04

LABEL maintainer="admin@sloong.com"
ENV ADDRESS_INFO=127.0.0.1:8001

RUN apt-get update \
    && apt-get install -y libsqlite3-0 libprotobuf10 libuuid1 libglib2.0-0 libssl1.1 libboost-serialization1.65.1 liblua5.3-0 libjsoncpp1 \
    && mkdir /app && mkdir /data
WORKDIR  /app

ADD output.tar /app/
ADD ./run.sh /app/
RUN chmod +x sloongnet run.sh

VOLUME /data
EXPOSE 8001
ENTRYPOINT ["/app/run.sh"]

