#!/bin/bash

# Script start: setup-wiringpi.sh.
cd ~

# Downloading the wiringpi library.
git clone git://git.drogon.net/wiringPi
cd wiringPi
git pull origin

# Building the wiringpi library.
./build

# Script finished: setup-wiringpi.sh.
cd ~
