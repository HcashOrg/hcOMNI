#!/bin/sh

#brew install automake berkeley-db libtool boost (miniupnpc) openssl pkg-config protobuf libevent
#before running this file, you should install the pkgs above on mac os.

chmod 777 autogen.sh

./autogen.sh
./configure --without-gui --without-miniupnpc --with-incompatible-bdb --disable-wallet --disable-tests --disable-bench

sudo make clean
sudo make
cd src
sudo ar rc libomnicored.a omnicored-bitcoind.o omnicored-omnicoreApi.o
cd ..

source libsCopy.sh
