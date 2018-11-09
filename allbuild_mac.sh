#!/bin/sh

chmod 777 autogen.sh

./autogen.sh
./configure --without-gui

make clean
make

sh ./libsCopy.sh