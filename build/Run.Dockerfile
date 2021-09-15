FROM debian:11-slim

# chagne to ustc source
RUN sed -i "s/deb.debian.org/mirrors.tuna.tsinghua.edu.cn/g" /etc/apt/sources.list

RUN apt-get update && apt install -y apt-transport-https ca-certificates

RUN sed -i "s/http/https/g" /etc/apt/sources.list

# copy file to docker
COPY . /tmp/
WORKDIR /tmp

# install build packages
RUN /tmp/build/environment.sh --run

