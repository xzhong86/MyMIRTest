#!/bin/sh

MIR=../mir

g++ -I$MIR $* -L$MIR -lmir

