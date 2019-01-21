#!/bin/bash

DIR=output/final

cd $DIR 

#
# apexcname.wb.sidnlabs.nl will fail...
#
for a in $(ls *.nl)
do
	named-compilezone -i "none" -s "relative" -f text -F text -o $a.txt $a $a
	mv $a.txt $a	
done


