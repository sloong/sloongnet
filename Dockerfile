FROM sloong/sloongnet_build AS build-env

# copy file to docker
COPY . /tmp/
WORKDIR /tmp

# start build
RUN /tmp/build/build.sh -r


FROM sloong/sloongnet_run
LABEL maintainer="admin@sloong.com"

RUN mkdir /app
WORKDIR /app
COPY --from=build-env /tmp/build/release/ /app
RUN chmod +x /app/sloongnet

RUN mkdir -p /data/log
ENV DB_FILE_PATH="/data/configuration.db"

VOLUME /data
EXPOSE 8000
ENTRYPOINT ["/app/sloongnet"]
CMD ["Worker","controller:8000"]
