#!/bin/bash

# Go to the correct directory
dirPATH=$(dirname $(dirname $(readlink -f $0)))
cd ${dirPATH}
make

# Threading primitive tests
(echo Benchmark,Count,Avg,Median,Min,99%,99.9%,99.99%,Max ;
bin/ArachneCreateTest | tail -n 1 ;
bin/NullYieldTest | tail -n 1 ;
bin/ArachneYieldTest | tail -n 1 ;
bin/ArachneCVTest | tail -n 1;
bin/ArachneBlockSignalTest | tail -n 1;
bin/ThreadTurnaround | tail -n 1 ) | scripts/column.py -s,


# Arbiter tests
if [[ "$1" == "--runArbiterBenchmarks" ]]; then
echo
(echo Benchmark,Count,Avg,Median,Min,99%,99.9%,99.99%,Max ;
bin/CoreRequest_Contended_Latency | tail -n 1;
bin/CoreRequest_Noncontended_Latency | tail -n 1) | scripts/column.py -s,
fi
# bin/CoreRequest_Contended_Timeout_Latency | tail -n 1;
