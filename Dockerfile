FROM sloong/sloongnet_build AS build-env

# copy file to docker
COPY . /tmp/
WORKDIR /tmp

# start build
RUN /tmp/build/build.sh -r


FROM sloong/sloongnet_run
LABEL maintainer="admin@sloong.com"

WORKDIR /usr/local/bin
COPY --from=build-env /tmp/build/release/ /usr/local/bin/
RUN chmod +x /usr/local/bin/sloongnet

RUN mkdir -p /data/log
ENV DB_FILE_PATH="/data/configuration.db"

VOLUME /data
EXPOSE 8000
ENTRYPOINT ["/usr/local/bin/sloongnet"]
CMD ["Worker","controller:8000"]
