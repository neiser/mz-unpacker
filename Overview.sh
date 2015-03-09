#!/bin/bash
for file in data/*calibrated.root
do
	echo $file
	root -b -q 'Test.cpp("'"$file"'")'
done