FROM python:3-alpine
LABEL maintainer="admin@sloong.com"

ENV PYTHONUNBUFFERED 1
RUN pip3 install django protobuf \
    && mkdir /app && mkdir /data
WORKDIR /app

ADD output.tar.gz /app/
RUN chmod +x /app/run.sh

VOLUME /data
EXPOSE 8000
ENTRYPOINT [ "/app/run.sh"]