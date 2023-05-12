#!/bin/bash
cd ..
cd bin || exit

./test_swapIn
./test_common
./test_lockfreequeue
./test_timer
./test_spanMutex