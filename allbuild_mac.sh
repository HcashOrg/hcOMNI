#!/bin/sh

#brew install automake berkeley-db libtool boost (miniupnpc) openssl pkg-config protobuf libevent
#before running this file, you should install the pkgs above on mac os. 
#if cmd "source libsCopy.sh" error,please run it manually.

chmod 777 autogen.sh

./autogen.sh
./configure --without-gui --without-miniupnpc --with-incompatible-bdb --disable-tests --disable-bench --disable-zmq

sudo make clean
sudo make
cd src
sudo ar rc libomnicored.a omnicored-bitcoind.o omnicored-omnicoreApi.o
cd ..

source libsCopy.sh

