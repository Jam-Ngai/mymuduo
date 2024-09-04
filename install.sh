#!/bin/bash

set -e

if [ ! -d /usr/include/mymuduo ]; then
  mkdir /usr/include/mymuduo
fi

cd $(pwd)/net
for header in $(ls *.h); do
  cp $header /usr/include/mymuduo
done
cd ..

if [ ! -d /usr/lib/mymuduo ]; then
  mkdir /usr/lib/mymuduo
fi

cp $(pwd)/lib/libmymuduo.so /usr/lib/mymuduo

export LD_LIBRARY_PATH=/usr/lib/mymuduo:$LD_LIBRARY_PATH

ldconfig

echo "Build completed!"
