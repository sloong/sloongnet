FROM debian:10-slim

LABEL maintainer="admin@sloong.com"
ENV ADDRESS_INFO=0.0.0.0:8001
ENV NODE_TYPE=Manager

RUN mkdir /app && mkdir /data && apt update && apt install -y wget unzip
WORKDIR  /app

RUN wget -o next_gen.tar.gz https://git.sloong.com:8000/sloongnet/engine/archive/next_gen.zip
RUN unzip next_gen.zip -d /app/
RUN /app/engine/build/environment.sh
RUN /app/engine/build/build.sh -r

FROM debian:10-slim

COPY --from=0 /app/engine/build/sloongnet-release /usr/local/bin
RUN chmod +x /usr/local/bin/sloongnet /usr/local/bin/run.sh

VOLUME /data
EXPOSE 8001
ENTRYPOINT ["/usr/local/bin/run.sh"]

