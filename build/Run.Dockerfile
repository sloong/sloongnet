FROM debian:11-slim

# chagne to ustc source
RUN apt update && apt install -y \
    apt-transport-https ca-certificates \
 && apt clean \
 && apt autoremove --yes \
 && rm -rf /var/lib/apt/lists/*


RUN sed -i "s/deb.debian.org/mirrors.tuna.tsinghua.edu.cn/g" /etc/apt/sources.list \
 && sed -i "s/security.debian.org/mirrors.tuna.tsinghua.edu.cn/g" /etc/apt/sources.list \
 && sed -i "s/http/https/g" /etc/apt/sources.list \
 && apt update 

# copy file to docker
COPY . /tmp/
WORKDIR /tmp

# install build packages
RUN /tmp/build/environment.sh --run \
 && apt clean \
 && apt autoremove --yes \
 && rm -rf /var/lib/apt/lists/* \
 && rm -rdf /tmp \
 && mkdir /tmp
