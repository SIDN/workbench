#!/bin/bash

DIR=output/final

cd $DIR 

#
# apexcname.wb.sidnlabs.nl will fail...
#
for a in $(ls *.nl)
do

	if [[ $a == *"apexcname.wb.sidnlabs.nl"* ]]; then
		# Prepocess the problematic one by applying a little trick
  		cp $a $a.prep
  		sed -i "s/\tCNAME\t/\tMB\t/" ./$a.prep
  		sed -i "s/\ CNAME\ /\ MB\ /" ./$a.prep
  		named-compilezone -i "none" -s "relative" -f text -F text -o $a.txt $a $a.prep
	else
  		named-compilezone -i "none" -s "relative" -f text -F text -o $a.txt $a $a
	fi


	if [[ $a == *"apexcname.wb.sidnlabs.nl"* ]]; then
		# Completing the problematic one
  		sed -i "s/\tMB\t/\tCNAME\t/" ./$a.txt
  		sed -i "s/\ MB\ /\ CNAME\ /" ./$a.txt
  		rm $a.prep
	fi

	mv $a.txt $a	
done
