#!/bin/bash
mkdir -p out
rm -f out/$1.out
timeout 120s ./main instances/$1.in out/$1.out
pkill lingeling
./checker instances/$1.in out/$1.out
