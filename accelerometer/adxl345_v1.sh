#!/bin/bash

gcc adxl345_v1.c -o adxl345_v1.o -lwiringPi

./adxl345_v1.o

gnuplot -e "plot './data.dat' using 1:2 title 'X' with line, './data.dat' using 1:3 title 'Y' with line, './data.dat' using 1:4 title 'Z' with line; pause(-1)"
