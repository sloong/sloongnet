FROM python:3-alpine

ENV PYTHONUNBUFFERED 1
RUN pip3 install django
RUN mkdir /WebUI
WORKDIR /WebUI
ADD output.tar.gz /WebUI/
RUN ls
EXPOSE 8000
CMD [ "run.sh"]