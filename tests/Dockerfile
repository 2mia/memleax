FROM alpine:edge

RUN apk update 
RUN apk add cmake 
RUN apk add gcc g++ make
RUN apk add musl-dev
RUN apk add libdwarf-dev libunwind-dev libelf-dev

ADD . /src
WORKDIR /
RUN cd /src && mkdir -p build 

#ENV PKG_CONFIG_PATH /usr/lib/pkgconfig
RUN cd /src/build/ && cmake .. && make

RUN apk add nginx curl && mkdir /run/nginx/
