FROM python:3-alpine

ENV PYTHONUNBUFFERED 1
RUN pip3 install django
RUN mkdir /WebUI
WORKDIR /WebUI
COPY output.tar.gz /WebUI/
RUN python3 -m pip install /WebUI/output.tar.gz
EXPOSE 8000
CMD [ "python3","manage.py","runserver" ]