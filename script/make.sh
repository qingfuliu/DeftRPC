#!/bin/bash
cd ..
mkdir build

cd build || exit
cmake ..
make
