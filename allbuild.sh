#!/bin/sh

chmod 777 autogen.sh
chmod 777 share/genbuild.sh
chmod 777 libsCopy.sh

sudo apt-get install make  pkg-config autoconf libtool
sudo apt-get install libboost-all-dev libssl-dev libevent-dev libdb-dev  libdb++-dev

./autogen.sh
./configure --with-incompatible-bdb --disable-tests --disable-bench
make clean
make
echo "copying ..."
sh ./libsCopy.sh