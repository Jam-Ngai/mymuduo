#!/bin/bash

set -e

if [! -d 'pwd'/build]; then
  mkdir 'pwd'/build
fi

rm -rf 'pwd'/build/*

cd 'pwd'/build && cmake .. && make

cd ..

if [! -d /usr/include/mymuduo/]; then
  mkdir /usr/include/mymuduo/
fi

for header in 'ls *.h'; do
  cp $header /usr/include/mymuduo/
done

if [! -d /usr/lib/mymuduo/]; then
  mkdir /usr/lib/mymuduo/
fi

cp 'pwd'/lib/libmymuduo /usr/lib/mymuduo

export LD_LIBRARY_PATH=/usr/lib/mymuduo:$LD_LIBRARY_PATH

ldconfig
