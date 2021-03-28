#! /bin/bash

check_result() {
	# $1 is always the return code.
	# $2 is always the test number.
	# $3 is always the invalid line.
	
	if [ $1 -eq 0 ] 
	then
		echo "FAILED: Test #$2 ($3), exit code was $1" >&2
	else
		echo "PASS: Test #$2 ($3)" >&2
	fi
}

codefile='../src/greplike'
filename='files/invalid_expr.txt'
matchfile='files/dummy1.txt'
tn=1

while read -r line; do
	./$codefile "$line" "$matchfile" > /dev/null 2>&1
	check_result $? $tn $line
	tn=$((tn+1))
done < $filename

# Check for invalid filenames.
filenames=('/' '\0' 'randomfile', 'files/noaccess.txt')
for f in ${filenames[@]}; do
	./$codefile "expr" "$f" > /dev/null 2>&1
	check_result $? $tn $f
	tn=$((tn+1))
done 

# Check for invalid options
options=('-!' '-z' '-A')
for o in ${options[@]}; do
	./$codefile "expr" "$matchfile" "$o" > /dev/null 2>&1
	check_result $? $tn $o
	tn=$((tn+1))
done

