#!/bin/sh
cd "$(dirname "$0")"
cd build
if [ $? -eq 0 ];then
    make -j8
else
    echo 'please ./build.sh'
fi
