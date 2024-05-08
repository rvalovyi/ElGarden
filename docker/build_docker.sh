#!/bin/bash
#TODO: Add check for the existing image, to avoid removing non exist image
case $1 in
  --clean)
        docker rmi -f raspberry/pi
    ;;

  *)
        docker build . -t raspberry/pi --network host
    ;;
esac


