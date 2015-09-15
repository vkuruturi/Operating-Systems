#!/bin/bash
#Script to test copycat with different buffer sizes.  
for i in `seq 1 10`;
do
	let 'z = 2**i'
	#echo $z
	/usr/bin/time --append --output=times.txt -f "%E %U %S" ./copycat.exe -b $z -o output.txt sample.txt
done