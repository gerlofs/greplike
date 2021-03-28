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
filename='files/file_test.txt'
tn=1

while mapfile -t -n 2 ary && ((${#ary[@]})); do
	./$codefile "${ary[1]}" "files/dummy${ary[0]}.txt" > /dev/null 2>&1
	check_result $? $tn ${ary[1]}
	tn=$((tn+1))
done < $filename
