FROM gcc:10.1

LABEL maintainer="admin@sloong.com"
ENV ADDRESS_INFO=0.0.0.0:8001
ENV NODE_TYPE=Manager

RUN apt update && apt install -y \
    unzip cmake libsqlite3-dev libprotobuf-dev protobuf-compiler uuid-dev libssl-dev libjsoncpp-dev libmariadbclient-dev libluajit-5.1-dev


RUN wget -o next_gen.tar.gz https://git.sloong.com:8000/sloongnet/engine/archive/next_gen.zip && \
    unzip next_gen.zip -d /app/ && \
    /app/engine/build/build.sh -r

FROM debian:10-slim

RUN apt update && apt install -y \
    libsqlite3-0 libprotobuf17 libuuid1 libssl1.1  libjsoncpp1 mariadb-client libluajit-5.1

COPY --from=0 /app/engine/build/sloongnet-release /usr/local/bin
RUN chmod +x /usr/local/bin/sloongnet /usr/local/bin/run.sh

RUN mkdir /data
VOLUME /data
EXPOSE 8000
ENTRYPOINT ["/usr/local/bin/run.sh"]

