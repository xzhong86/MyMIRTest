#!/bin/sh

MIR=../mir
MIR_INC=$MIR
MIR_LIB=$MIR/build

echo "g++ -I$MIR_INC $* -L$MIR_LIB -lmir"
g++ -I$MIR_INC $* -L$MIR_LIB -lmir -lpthread

