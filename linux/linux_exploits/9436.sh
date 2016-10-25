#!/bin/sh

gcc -o run run.c && \
gcc -o exploit exploit.c && \
./run
