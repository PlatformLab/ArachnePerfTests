#!/bin/bash

# Go to the correct directory
dirPATH=$(dirname $(dirname $(readlink -f $0)))
cd ${dirPATH}
make

# Threading primitive tests
(echo Benchmark,Count,Avg,Median,Min,99%,99.9%,99.99%,Max ;
bin/ArachneCreateTest $HTACTIVE | tail -n 1 ;
bin/NullYieldTest  $HTACTIVE | tail -n 1 ;
bin/ArachneYieldTest $HTACTIVE | tail -n 1 ;
bin/ArachneCVTest $HTACTIVE | tail -n 1;
bin/ThreadTurnaround $HTACTIVE | tail -n 1 ;
bin/ArachneBlockSignalTest $HTACTIVE | tail -n 1 ) 2>/dev/null | scripts/column.py -s,


# Arbiter tests
if [[ "$1" == "--runArbiterBenchmarks" ]]; then
echo
(echo Benchmark,Count,Avg,Median,Min,99%,99.9%,99.99%,Max ;
bin/CoreRequest_Contended_Latency | tail -n 1;
bin/CoreRequest_Noncontended_Latency | tail -n 1) 2>/dev/null | scripts/column.py -s,
fi
# bin/CoreRequest_Contended_Timeout_Latency | tail -n 1;
