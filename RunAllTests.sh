#!/bin/bash

echo Benchmark,Count,Avg,Median,Min,99%,99.9%,99.99%,Max
./ArachneCreateTest | tail -n 1
./NullYieldTest | tail -n 1
./ArachneYieldTest | tail -n 1
./ArachneCVTest | tail -n 1
./ArachneBlockSignalTest | tail -n 1
