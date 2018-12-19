FROM ubuntu

RUN apt-get update
RUN apt-get install -y cmake 
RUN apt-get install -y gcc g++ make
RUN apt-get install -y libunwind-dev libelf-dev
RUN apt-get install -y pkg-config

ADD . /src
WORKDIR /
RUN cd /src && mkdir -p build 

#ENV PKG_CONFIG_PATH /usr/lib/pkgconfig
RUN cd /src/build/ && cmake .. && make

#RUN apk add nginx curl && mkdir /run/nginx/
