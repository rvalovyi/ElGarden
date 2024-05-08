#!/bin/bash

echo "Compile with UID=${LOCAL_USER_ID} GID=${LOCAL_GROUP_ID} HOME=${LOCAL_HOME_PATH}"

case $1 in
  --arm)
        echo "--arm"
        docker container run \
        -v $(pwd):/home/pi/ \
        --network host \
        -e "LOCAL_USER_ID=${LOCAL_USER_ID}" \
        -e "LOCAL_USER_NAME=${LOCAL_USER_NAME}" \
        -e "LOCAL_GROUP_ID=${LOCAL_GROUP_ID}" \
        -e "LOCAL_HOME_PATH=${LOCAL_HOME_PATH}" \
            raspberry/pi \
        sh -c '/home/pi/cmpl.sh --arm'
        result=$?
    ;;

  --clean)
        echo "--clean"
        docker container run \
        -v $(pwd):/home/pi \
        --network host \
        -e "LOCAL_USER_ID=${LOCAL_USER_ID}" \
        -e "LOCAL_USER_NAME=${LOCAL_USER_NAME}" \
        -e "LOCAL_GROUP_ID=${LOCAL_GROUP_ID}" \
        -e "LOCAL_HOME_PATH=${LOCAL_HOME_PATH}" \
            raspberry/pi \
        sh -c '/home/pi/cmpl.sh --clean'
        result=$?
    ;;

  *)
        echo "--init - Init cross-compile."
        echo "--arm - cross-compile release for ARM platform."
        echo "--clean - cleans all the build files in the project"
    ;;
esac

if [[ $result != 0 ]]
then
    exit 1 # terminate if an error occurs
fi
