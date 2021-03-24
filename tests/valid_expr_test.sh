#! /bin/bash

# valid_expr_tests.sh
# A text file is used with an expression followed by an integer value (seperated by a line break)
# 	that determines whether the regular expression should match, this is used with check_
#	result to pass or fail the test.

check_result() {
	# $1 is always the return code.
	# $2 is always the test number.
	# $3 is always the invalid line.
	
	if [ $1 -ne 0 ] 
	then
		echo "FAILED: Test #$2 {$3}, exit code was $1" >&2
	else 
		echo "PASSED: Test #$2 {$3}" >&1
	fi
}

codefile='../src/greplike'
filename='valid_expr.txt'
matchfile='dummy.txt'
tn=1

while read -r line; do
	./$codefile "$line" "$matchfile"  #>> results.txt #/dev/null 2>&1
	check_result $? $tn $line
	tn=$((tn+1))
done < $filename
