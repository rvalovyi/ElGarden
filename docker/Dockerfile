FROM debian:10

MAINTAINER Ruslan Valovyi
ENV GIT_REPO el_garden

#Build Debian dependencies
RUN apt-get -y update && apt-get -y upgrade && apt-get install -y build-essential \
    -y git \
    -y gettext \
    -y flex \
    -y bison \
    -y autoconf \
    -y libtool \
    -y automake \
    -y pkg-config \
    -y cmake \
    -y gawk \
    -y gcc \
    -y g++ \
    -y gperf \
    -y flex \
    -y texinfo  \
    -y gfortran \
    -y build-essential \
    -y openssl \ 
    -y unzip \
    -y wget \
    -y pigz \
    -y libncurses-dev \
    -y tar \
    -y figlet \
    -y ccache \
    -y rsync

#TODO get gcc compilator version from file ~/cross-gcc-*.tar.gz and use it instaead of 'cross-pi-gcc-10.3.0-2'
RUN /usr/sbin/update-ccache-symlinks \
    && echo 'export PATH="/usr/lib/ccache:$PATH"' | tee -a ~/.bashrc \
    && export CCACHE_COMPRESS=1 \
    && export CCACHE_DIR=$HOME/.ccache  \
    && . ~/.bashrc && echo $PATH \
    && cd /tmp && git clone https://github.com/abhiTronix/raspberry-pi-cross-compilers.git \
    && cd /tmp/raspberry-pi-cross-compilers/build-scripts && chmod +x ./RTBuilder_32b && ./RTBuilder_32b -g "10.3.0" -r "3+" -o "buster" -V \
    && mkdir -p /opt/cross \
    && tar -xzvf ~/cross-gcc-*.tar.gz -C /opt/cross \
    && rm -rf ~/*.tar.gz \
    && cd / && rm -rf /tmp/*

COPY Toolchain-RaspberryPi.cmake /opt/cross/Toolchain-RaspberryPi.cmake
ENV PATH "$PATH:/opt/cross/cross-pi-gcc-10.3.0-2/bin:/opt/cross/cross-pi-gcc-10.3.0-2/lib:/opt/cross/cross-pi-gcc-10.3.0-2/include"

RUN cd /tmp && git clone https://github.com/guillemj/libmd.git \
    && cd /tmp/libmd && ./autogen && ./configure --host=arm-linux-gnueabihf && make \
    && cp -P /tmp/libmd/src/.libs/libmd.a /opt/cross/cross-pi-gcc-10.3.0-2/lib/ \
    && cp -P /tmp/libmd/src/.libs/libmd.so* /opt/cross/cross-pi-gcc-10.3.0-2/lib/ \
    && cp /tmp/libmd/include/md5.h /opt/cross/cross-pi-gcc-10.3.0-2/include/ \
    && cd / && rm -rf /tmp/*

RUN cd /tmp && git clone https://gitlab.freedesktop.org/libbsd/libbsd.git \
    && cd /tmp/libbsd && ./autogen \
    && ./configure --host=arm-linux-gnueabihf LDFLAGS=-L/opt/cross/cross-pi-gcc-10.3.0-2/lib CFLAGS=-I/opt/cross/cross-pi-gcc-10.3.0-2/include \
    && make \
    && cp -P /tmp/libbsd/src/.libs/libbsd.a /opt/cross/cross-pi-gcc-10.3.0-2/lib/ \
    && cd / && rm -rf /tmp/*


RUN cd /tmp && git clone https://github.com/json-c/json-c.git \
    && cd /tmp/json-c && cmake -B json-c-build -DCMAKE_TOOLCHAIN_FILE=/opt/cross/Toolchain-RaspberryPi.cmake  \
    && cd /tmp/json-c/json-c-build && make  \
    && cp -P /tmp/json-c/json-c-build/libjson-c.a /opt/cross/cross-pi-gcc-10.3.0-2/lib/ \
    && mkdir -p /opt/cross/cross-pi-gcc-10.3.0-2/include/json-c \
    && cp /tmp/json-c/json-c-build/json.h /opt/cross/cross-pi-gcc-10.3.0-2/include/json-c \
    && cp /tmp/json-c/json-c-build/json_config.h /opt/cross/cross-pi-gcc-10.3.0-2/include/json-c \
    && cp /tmp/json-c/arraylist.h /opt/cross/cross-pi-gcc-10.3.0-2/include/json-c \
    && cp /tmp/json-c/debug.h /opt/cross/cross-pi-gcc-10.3.0-2/include/json-c  \
    && cp /tmp/json-c/json_c_version.h /opt/cross/cross-pi-gcc-10.3.0-2/include/json-c \
    && cp /tmp/json-c/json_object.h /opt/cross/cross-pi-gcc-10.3.0-2/include/json-c  \
    && cp /tmp/json-c/json_inttypes.h /opt/cross/cross-pi-gcc-10.3.0-2/include/json-c \
    && cp /tmp/json-c/json_types.h /opt/cross/cross-pi-gcc-10.3.0-2/include/json-c \
    && cp /tmp/json-c/printbuf.h /opt/cross/cross-pi-gcc-10.3.0-2/include/json-c  \
    && cp /tmp/json-c/json_object_iterator.h /opt/cross/cross-pi-gcc-10.3.0-2/include/json-c \
    && cp /tmp/json-c/json_patch.h /opt/cross/cross-pi-gcc-10.3.0-2/include/json-c \
    && cp /tmp/json-c/json_pointer.h /opt/cross/cross-pi-gcc-10.3.0-2/include/json-c \
    && cp /tmp/json-c/json_tokener.h /opt/cross/cross-pi-gcc-10.3.0-2/include/json-c \
    && cp /tmp/json-c/json_util.h /opt/cross/cross-pi-gcc-10.3.0-2/include/json-c \
    && cp /tmp/json-c/json_visit.h /opt/cross/cross-pi-gcc-10.3.0-2/include/json-c \
    && cp /tmp/json-c/linkhash.h /opt/cross/cross-pi-gcc-10.3.0-2/include/json-c \
    && cd / && rm -rf /tmp/*


RUN cd /tmp && git clone https://github.com/joan2937/pigpio.git \
    && cd /tmp/pigpio && make CROSS_PREFIX=arm-linux-gnueabihf- \
    && cp -P /tmp/pigpio/libpigpio.so* /opt/cross/cross-pi-gcc-10.3.0-2/lib/  \
    && cp -P /tmp/pigpio/libpigpiod_if*.so* /opt/cross/cross-pi-gcc-10.3.0-2/lib/  \
    && cp /tmp/pigpio/pigpio.h /opt/cross/cross-pi-gcc-10.3.0-2/include/ \
    && cd / && rm -rf /tmp/*