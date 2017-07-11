#!/bin/bash

echo "Occupied Thread Slots,Median Latency,99% Latency"
for i in $(seq 0 55); do
    echo -n $i,
    bin/ArachneBlockSignalTest $i| tail -n 2 | awk -F, 'NR==2{print $4 "," $6}'
done
