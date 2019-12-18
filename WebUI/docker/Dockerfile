FROM python:3-alpine

ENV PYTHONUNBUFFERED 1
RUN pip3 install django protobuf
RUN mkdir /WebUI
ADD output.tar.gz /WebUI/
RUN chmod +x /WebUI/run.sh
WORKDIR /WebUI
EXPOSE 8000
ENTRYPOINT [ "/WebUI/run.sh"]