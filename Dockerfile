FROM debian:11-slim AS build-env

# chagne to ustc source
RUN sed -i "s/deb.debian.org/mirrors.ustc.edu.cn/g" /etc/apt/sources.list

# install build packages
RUN ./build/environment.sh --build

# copy file to docker
COPY . /tmp/
WORKDIR /tmp

# start build
RUN /tmp/build/build.sh -r


FROM debian:11-slim
LABEL maintainer="admin@sloong.com"

# chagne to ustc source
RUN sed -i "s/deb.debian.org/mirrors.ustc.edu.cn/g" /etc/apt/sources.list

# install runtime packages
RUN ./build/environment.sh --run

WORKDIR /usr/local/bin
COPY --from=build-env /tmp/build/release/ /usr/local/bin/
RUN chmod +x /usr/local/bin/sloongnet

RUN mkdir -p /data/log
VOLUME /data
EXPOSE 8000
ENTRYPOINT ["/usr/local/bin/sloongnet"]
CMD ["Worker","controller:8000"]
