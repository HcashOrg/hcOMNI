#!/bin/sh

cd src
ar rc libomnicored.a omnicored-bitcoind.o omnicored-omnicoreApi.o 
cp univalue/.libs/libunivalue.a $GOPATH/src/github.com/HcashOrg/hcwallet/omnilib -rf
cp crypto/libbitcoin_crypto.a $GOPATH/src/github.com/HcashOrg/hcwallet/omnilib -rf
cp leveldb/libleveldb.a $GOPATH/src/github.com/HcashOrg/hcwallet/omnilib -rf
cp leveldb/libmemenv.a $GOPATH/src/github.com/HcashOrg/hcwallet/omnilib -rf
cp secp256k1/.libs/libsecp256k1.a $GOPATH/src/github.com/HcashOrg/hcwallet/omnilib -rf
cp *.a  $GOPATH/src/github.com/HcashOrg/hcwallet/omnilib -rf
