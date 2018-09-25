#!/bin/bash

# Go to the correct directory
dirPATH=$(dirname $(dirname $(readlink -f $0)))
cd ${dirPATH}

# Do one run to warm up the core arbiter
bin/ArachneBlockSignalTest 2> /dev/null >&2

echo "Occupied Thread Slots,Median Latency,99% Latency"
for i in $(seq 0 55); do
    echo -n $i,
    bin/ArachneBlockSignalTest -s $i| tail -n 2 | awk -F, 'NR==2{print $4 "," $6}'
done
