for d in "$(ls /opt/cross)"; do if [[ $d == cross-pi-gcc* ]] ; then  echo $d; fi; done
