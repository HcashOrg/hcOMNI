#!/bin/sh

chmod 777 autogen.sh
chmod 777 share/genbuild.sh

sudo apt-get install make  pkg-config autoconf libtool
sudo apt-get install libboost-all-dev libssl-dev libevent-dev libdb-dev  libdb++-dev

autogen.sh
configure --with-incompatible-bdb --disable-tests --disable-bench
make clean
make

cd src
ar rc libomnicored.a omnicored-bitcoind.o omnicored-omnicoreApi.o 
cp univalue/.libs/libunivalue.a $GOPATH/src/github.com/HcashOrg/hcwallet/omnilib -rf
cp crypto/libbitcoin_crypto.a $GOPATH/src/github.com/HcashOrg/hcwallet/omnilib -rf
cp leveldb/libleveldb.a $GOPATH/src/github.com/HcashOrg/hcwallet/omnilib -rf
cp leveldb/libmemenv.a $GOPATH/src/github.com/HcashOrg/hcwallet/omnilib -rf
cp secp256k1/.libs/libsecp256k1.a $GOPATH/src/github.com/HcashOrg/hcwallet/omnilib -rf
cp *.a  $GOPATH/src/github.com/HcashOrg/hcwallet/omnilib -rf
cd..

