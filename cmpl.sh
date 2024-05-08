#!/bin/bash

BUILD_DIR=build

if [[ $1 = "--clean" ]]
then
    echo "cmpl.sh --clean"
    if rm -rf /home/pi/src/$BUILD_DIR/; then
        echo "Folder ./src/build/ was removed successfully."
    else
        echo "[WARN] Folder ./src/$BUILD_DIR/ wasn't removed."
        result=-1
    fi
    result=0
else
    echo "cmpl.sh --arm"

#    if [[ ! -f /home/pi/libs/libmd/src/.libs/libmd.a ]] ; then
#        # Build libmd module
#        cd /home/pi/libs/libmd
#        ./autogen
#        ./configure --host=arm-linux-gnueabihf 
#        make
#        result=$?
#        
#        if [[ $result != 0 ]]
#        then
#            exit 1 # terminate if an error occurs
#        fi
#    fi
#
#    if [[ ! -f /home/pi/libs/libbsd/src/.libs/libbsd.a ]] ; then
#        echo "libbsd ..."
#        cd /home/pi/libs/libbsd
#        ./autogen
#        ./configure --host=arm-linux-gnueabihf LDFLAGS=-L/home/pi/libs/libmd/src/.libs CFLAGS=-I/home/pi/libs/libmd/include
#        make
#        result=$?
#    
#        if [[ $result != 0 ]]
#        then
#            exit 1 # terminate if an error occurs
#        fi
#    fi
#
#    if [[ ! -f /home/pi/libs/json-c/json-c-build/libjson-c.a ]] ; then
#        echo "json-c ..."
#        cd /home/pi/libs/json-c
#        cmake -B json-c-build -DCMAKE_C_COMPILER=/opt/cross/cross-pi-gcc-10.3.0-2/bin/arm-linux-gnueabihf-gcc 
#        cd json-c-build
#        make
#        result=$?
#        
#        if [[ $result != 0 ]]
#        then
#            exit 1 # terminate if an error occurs
#        fi
#    fi
#
#    if [[ ! -f /home/pi/libs/pigpio/libpigpio.so ]] ; then
#        echo "pigpio ..."
#        cd /home/pi/libs/pigpio
#        make CROSS_PREFIX=/opt/cross/cross-pi-gcc-10.3.0-2/bin/arm-linux-gnueabihf-
#        result=$?
#        
#        if [[ $result != 0 ]]
#        then
#            exit 1 # terminate if an error occurs
#        fi
#    fi

    cd /home/pi

    if [ ! -d "$BUILD_DIR" ]; then
         cmake -B $BUILD_DIR -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=/opt/cross/Toolchain-RaspberryPi.cmake 
    else
         cmake --build $BUILD_DIR
    fi

    cd /home/pi/$BUILD_DIR
    make
    result=$?
fi
if [[ $result != 0 ]]
then
    exit 1 # terminate if an error occurs
fi
