#!/bin/sh

chmod 777 autogen.sh
#chmod 777 share/genbuild.sh
chmod 777 libsCopy.sh

sudo apt-get install make  pkg-config autoconf libtool
sudo apt-get install libboost-all-dev libssl-dev libevent-dev libdb-dev  libdb++-dev

./autogen.sh
./configure  --without-gui --without-miniupnpc --with-incompatible-bdb --disable-wallet --disable-tests --disable-bench

make clean
make

cd src
sudo ar rc libomnicored.a omnicored-bitcoind.o omnicored-omnicoreApi.o
cd ..
echo "copying ..."
sh ./libsCopy.sh
