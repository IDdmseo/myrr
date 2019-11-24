#!/bin/sh

echo "***** input record-replay mode (1. record: rc, 2. replay: rc) *****"
read mode
echo "***** input filename to record or replay *****"
read binary

LD_PRELOAD=./rr_library.so ./$binary
./monitor $mode ./$binary 5555

