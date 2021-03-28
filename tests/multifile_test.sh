#! /bin/bash

check_result() {
	# $1 is always the return code.
	# $2 is always the test number.
	# $3 is always the line.
	
	if [ $1 -ne 0 ] 
	then
		echo "FAILED: Test #$2 ($3), exit code was $1" >&2
	else
		echo "PASS: Test #$2 ($3)" >&2
	fi
}

codefile='../src/greplike'
exprs=('^[A-Za-z]' '[a-z]. $' 'e.n' '[A-Z](o|e|r)+ ')
tn=1

for e in "${exprs[@]}"
do
	./$codefile "$e" "-f" "files/dummy1.txt" "files/dummy2.txt" "files/dummy3.txt" "-n" "-i" "-c" > /dev/null 2>&1
	check_result $? $tn $e
	tn=$((tn+1))
done
