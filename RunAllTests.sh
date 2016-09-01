#!/bin/bash

GREEN='\033[0;32m'
NC='\033[0m'

echo -e $GREEN"Yield Test"$NC
make ArachneYieldTest && timeout 3 ./ArachneYieldTest
echo -e "\n\n"

./RunTest.sh CV "Producer about"
./RunTest.sh BlockSignal_ContextSwitch "Producer about to signal 0"
./RunTest.sh BlockSignal "Producer about to signal"
./RunTest.sh Create "A thread is about to be born"
# This test is run manually
