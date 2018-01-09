#!/bin/bash

# Go to the correct directory
dirPATH=$(dirname $(dirname $(readlink -f $0)))
cd ${dirPATH}

echo "Occupied Thread Slots,Median Latency,99% Latency"
for i in $(seq 0 55); do
    echo -n $i,
    bin/ArachneCreateTest $i | tail -n 2 | awk -F, 'NR==2{print $4 "," $6}'
done
