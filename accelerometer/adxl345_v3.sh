#!/bin/bash

gcc adxl345_v3.c -o adxl345_v3.o -lwiringPi

./adxl345_v3.o

gnuplot -e "plot './data.dat' using 1:2 title 'X' with line, './data.dat' using 1:3 title 'Y' with line, './data.dat' using 1:4 title 'Z' with line, './data_fir.dat' using 1:2 title 'X_fir' with line, './data_fir.dat' using 1:3 title 'Y_fir' with line, './data_fir.dat' using 1:4 title 'Z_fir' with line; pause(-1)"
