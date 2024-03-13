#!/bin/bash
if [ "$#" -ne 3 ];
then
  exit 1
fi
file=$1
foo=$2
bar=$3
sed -i s/${foo}/${bar}/g ${file}


