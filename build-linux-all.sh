#!/bin/bash
# 
# Author: Mark Gottscho
# mgottscho@ucla.edu

./build-linux.sh x64 &
./build-linux.sh x64_avx &
./build-linux.sh x86 &
./build-linux.sh arm &
./build-linux.sh arm_neon &
./build-linux.sh arm64 &
