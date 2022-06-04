#! /usr/bin/sh
cd External/Rabbitmqc
mkdir -p build
cd build
cmake ..
sudo cmake --build . --target install
cd ../../../
