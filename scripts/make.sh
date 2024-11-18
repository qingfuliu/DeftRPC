#!/bin/bash
cd ..
mkdir build

cd build || exit
git submodule init
git submodule update
cmake ..
make
