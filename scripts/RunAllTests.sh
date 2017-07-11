#!/bin/bash

make

(echo Benchmark,Count,Avg,Median,Min,99%,99.9%,99.99%,Max ;
bin/ArachneCreateTest | tail -n 1 ;
bin/NullYieldTest | tail -n 1 ;
bin/ArachneYieldTest | tail -n 1 ;
bin/ArachneCVTest | tail -n 1;
bin/ArachneBlockSignalTest | tail -n 1;
bin/ThreadTurnaround | tail -n 1 ) | scripts/column.py -s,
