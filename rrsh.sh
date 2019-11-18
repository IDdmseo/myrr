#!/bin/sh

echo "***** input record-replay mode *****"
read mode
echo "***** input filepath to record or replay *****"
read binary

LD_PRELOAD=./netlib.so ./$binary
./monitor $mode ./$binary


