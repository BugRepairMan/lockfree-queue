#!/bin/bash

echo "Hello World!"

cmd="./build/fifo_queue 2"

for run in {1..100}
do
	echo $run
	$cmd
done
