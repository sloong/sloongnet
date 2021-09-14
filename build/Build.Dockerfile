FROM debian:11-slim

# chagne to ustc source
RUN sed -i "s/deb.debian.org/mirrors.tuna.tsinghua.edu.cn/g" /etc/apt/sources.list

# copy file to docker
COPY . /tmp/
WORKDIR /tmp

# install build packages
RUN /tmp/build/environment.sh --build