#!/bin/bash
#First you can use grep (-n) to find the number of lines of string.
#Then you can use awk to separate the answer.

if [ "$#" -ne 3 ]; then
    exit 1
fi
foo=$1
s=$2
bar=$3

grep -n ${s} < ${foo} | cut -d ':' -f 1 > ${bar}
# or
# 
