#!/bin/bash

set -e

# Create the binaries directory
mkdir -p bin/

g++ src/main.cpp src/bmp.cpp -o bin/main.elf -std=c++0x
