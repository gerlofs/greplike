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

resulttime=
codefile='../src/greplike'
exprs=('^[A-Za-z]' '[a-z]. $' 'e.n' '[A-Z](o|e|r)+ ' '[a-z]*' '.*[a-z]+ ' '..*..' '[A-Z]+g.*[a-z].*' '(a|b|c|d|e|f|g|h|i|j|k|l|m|n|o|p|q|r|s|t|u|v|w|x|y|z)+')
tn=1

for e in "${exprs[@]}"
do
	stats=$({ time ./$codefile "$e" "files/dummy1.txt" "files/dummy2.txt" "files/dummy3.txt" 1>/dev/null 2>&1; } 2>&1)
	echo "Greplike: " $stats
	stats=$({ time grep "$e" "files/dummy1.txt" "files/dummy2.txt" "files/dummy3.txt" 1>/dev/null 2>&1; } 2>&1)
	echo "Grep: " $stats
	check_result $? $tn $e
	tn=$((tn+1))
done
