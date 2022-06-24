#! /usr/bin/sh

mkdir -p Build
git submodule update --init
sudo apt-get install libboost-all-dev
./install_dep.sh
cd Build

# Install libhv
git clone https://github.com/ithewei/libhv.git
cd libhv
./configure
make
sudo make install
cd .. # Leaving libhv/

# Install hiredis
git clone https://github.com/redis/hiredis.git
cd hiredis
make
sudo make install
sudo /sbin/ldconfig
cd .. # Leaving hiredis/

cd .. # leaving Build/

