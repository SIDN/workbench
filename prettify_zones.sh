#!/bin/bash

DIR=output/final

cd $DIR 

#
# apexcname.wb.sidnlabs.nl will fail...
#
for a in $(ls *.nl)
do
	# Prepocess the problematic one
	if [[ $a == *"apexcname.wb.sidnlabs.nl"* ]]; then
  		#echo "preprocessing $a"
  		cp $a $a.prep
  		sed -i "s/\tCNAME\t/\tMB\t/" ./$a.prep
  		sed -i "s/\ CNAME\ /\ MB\ /" ./$a.prep
  		named-compilezone -i "none" -s "relative" -f text -F text -o $a.txt $a $a.prep
	else
  		named-compilezone -i "none" -s "relative" -f text -F text -o $a.txt $a $a
	fi

	# Completing the problematic one
	if [[ $a == *"apexcname.wb.sidnlabs.nl"* ]]; then
  		#echo "finisching $a"
  		sed -i "s/\tMB\t/\tCNAME\t/" ./$a.txt
  		sed -i "s/\ MB\ /\ CNAME\ /" ./$a.txt
  		rm $a.prep
	fi

	mv $a.txt $a	
done
