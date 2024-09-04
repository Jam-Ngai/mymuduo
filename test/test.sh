#!/bin/bash

for i in $(seq 0 10000); do
  ./bin/client 127.0.0.1 8080 &
done
