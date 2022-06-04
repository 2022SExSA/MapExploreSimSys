#! /usr/bin/sh
mkdir -p Build
cd Build
cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=1
make
cd ..
