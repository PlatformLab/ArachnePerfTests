#!/bin/bash

GREEN='\033[0;32m'
NC='\033[0m'

echo -e "${GREEN}${1}Test${NC}"
make Arachne${1}Test  && rm -f ${1}.log && ( timeout  5 ./Arachne${1}Test --numCores 2 ) ; ttsum.py -f "$2" ${1}.log
echo -e "\n\n"
