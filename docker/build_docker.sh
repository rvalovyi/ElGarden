#!/bin/bash
#TODO: Add check for the existing image, to avoid removing non exist image
case $1 in
  --clean)
        docker rmi -f raspberry/pi
    ;;

  --debug)
        docker build . -t raspberry/pi --network host --progress plain --no-cache
    ;;

  *)
        docker build . -t raspberry/pi --network host 
    ;;
esac


