#!/bin/sh

export LD_LIBRARY_PATH=$PWD/../lib:$LD_LIBRARY_PATH
./kc_websrv ../../website/my-prj.xml

