#!/bin/bash

BUILD_DIR=build

if [[ $1 = "--clean" ]]
then
    if rm -rf /home/pi/$BUILD_DIR/; then
        echo "Folder /home/pi/$BUILD_DIR/ was removed successfully."
    else
        echo "[WARN] Folder /home/pi/$BUILD_DIR/ wasn't removed."
        result=-1
    fi
    result=0
else
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
