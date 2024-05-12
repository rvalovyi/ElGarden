#!/bin/bash

BUILD_DIR=build

if [[ $1 = "--clean" ]]
then
    if rm -rf /home/pi/src/$BUILD_DIR/; then
        echo "Folder ./src/build/ was removed successfully."
    else
        echo "[WARN] Folder ./src/$BUILD_DIR/ wasn't removed."
    fi
    result=0
else
    # Build aqos module
    cd /home/pi/src

    if [[ $1 = "--arm" ]]
    then
        if [ ! -d "$BUILD_DIR" ]; then
             cmake -B $BUILD_DIR -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=/opt/cross/Toolchain-RaspberryPi.cmake 
        else
             cmake --build $BUILD_DIR
        fi
    fi

    cd /home/pi/src/$BUILD_DIR
    make
    result=$?
fi
if [[ $result != 0 ]]
then
    exit 1 # terminate if an error occurs
fi
