SET(CMAKE_SYSTEM_NAME Linux)
SET(CMAKE_SYSTEM_VERSION 1)
SET(CMAKE_C_COMPILER /opt/cross/cross-pi-gcc-10.3.0-2/bin/arm-linux-gnueabihf-gcc)
SET(CMAKE_CXX_COMPILER /opt/cross/cross-pi-gcc-10.3.0-2/bin/arm-linux-gnueabihf-g++)
SET(CMAKE_FIND_ROOT_PATH /opt/cross)
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)