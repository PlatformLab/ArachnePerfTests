#!/bin/bash

make ArachneYieldTest && timeout 3 ./ArachneYieldTest
./RunTest.sh CV "Producer about"
./RunTest.sh BlockSignal_ContextSwitch "Producer about to signal 0"
./RunTest.sh BlockSignal "Producer about to signal"
./RunTest.sh Create "A thread is about to be born"
# This test is run manually
