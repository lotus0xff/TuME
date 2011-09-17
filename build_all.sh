#!/bin/sh

mkdir build
cd build
qmake ../Teumaq/Teumaq.pro
make && make clean
cd ../docs
doxygen


