#!/bin/bash

make Arachne${1}Test  && rm -f ${1}.log && ( timeout  3 ./Arachne${1}Test  ) ; ttsum.py -f "$2" ${1}.log
