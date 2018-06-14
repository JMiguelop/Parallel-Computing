#!/bin/sh

echo "Compiling..."

make

echo "Running the tests"

./bin/lab1

echo "You can now copy timings.dat"
