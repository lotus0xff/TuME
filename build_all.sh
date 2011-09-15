#!/bin/sh

cd build
qmake "CONFIG+=release" ../Teumaq/Teumaq.pro
make && make clean
cd ../docs
doxygen


