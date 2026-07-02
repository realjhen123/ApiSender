#!/bin/sh
cd "$(dirname "$0")"
rm -rf build/
mkdir build
cd build
cmake .. 
make -j8
cd ..
ln -s build/compile_commands.json
