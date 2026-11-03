#!/bin/sh

export LD_LIBRARY_PATH=$PWD/../lib:$LD_LIBRARY_PATH
./kc_websrv ../website/my-prj.xml
#setsid ./kc_websrv ../website/my-prj.xml > console.log 2>&1 < /dev/null &
