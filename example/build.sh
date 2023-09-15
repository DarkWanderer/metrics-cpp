#!/bin/bash

g++ example.cpp -o example -I../include -I../build/src -L../build -lmetrics

LD_LIBRARY_PATH=../build ./example