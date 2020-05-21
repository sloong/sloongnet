FROM ubuntu:18.04

LABEL maintainer="admin@sloong.com"
ENV ADDRESS_INFO=0.0.0.0:8001
ENV NODE_TYPE=Manager

RUN apt-get update \
    && apt-get install -y libsqlite3-0 libprotobuf17 libuuid1 libssl1.1 libboost-serialization1.67.0 liblua5.3-0 libjsoncpp1 \
    && mkdir /app && mkdir /data
WORKDIR  /app

ADD output.tar /app/
ADD ./run.sh /app/
RUN chmod +x sloongnet run.sh

VOLUME /data
EXPOSE 8001
ENTRYPOINT ["/app/run.sh"]

