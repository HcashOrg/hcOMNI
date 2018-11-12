#!/bin/sh

cd src
#ar rc libomnicored.a omnicored-bitcoind.o omnicored-omnicoreApi.o 
cp -rf univalue/.libs/libunivalue.a $GOPATH/src/github.com/HcashOrg/hcwallet/omnilib
cp -rf crypto/libbitcoin_crypto.a $GOPATH/src/github.com/HcashOrg/hcwallet/omnilib
cp -rf leveldb/libleveldb.a $GOPATH/src/github.com/HcashOrg/hcwallet/omnilib
cp -rf leveldb/libmemenv.a $GOPATH/src/github.com/HcashOrg/hcwallet/omnilib
cp -rf secp256k1/.libs/libsecp256k1.a $GOPATH/src/github.com/HcashOrg/hcwallet/omnilib
cp -rf *.a  $GOPATH/src/github.com/HcashOrg/hcwallet/omnilib
