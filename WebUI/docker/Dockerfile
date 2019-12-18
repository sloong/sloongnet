FROM python:3-alpine
LABEL maintainer="admin@sloong.com"

ENV PYTHONUNBUFFERED 1
RUN pip3 install django protobuf \
    && mkdir /WebUI && mkdir /data
WORKDIR /WebUI

ADD output.tar.gz /WebUI/
RUN chmod +x /WebUI/run.sh

VOLUME /data
EXPOSE 8000
ENTRYPOINT [ "/WebUI/run.sh"]