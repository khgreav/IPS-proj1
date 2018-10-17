#!/bin/bash
if [ -z $1 ];
then
    max=1000;
else
    max=$1;
fi

passed=0;
failed=0;

for i in `seq 1 $max`
do
    ./psed '(.*) tohle (.*)' '$2 XX $1' 'je ' 'byl ' < test.in > tmp_soubor
    DIFF1=$(diff test1.out tmp_soubor)
    ./psed '(.*) tohle (.*)' '$2 XX $1' 'je ' 'byl ' '23' '211' < test.in > tmp_soubor
    DIFF2=$(diff test2.out tmp_soubor)
    if [ "$DIFF1" != "" -o "$DIFF2" != "" ]
    then
        echo "[ OK ] - $i/$max"
        passed=$((passed+1))
    else
        failed=$((failed+1))
    fi
done
echo "tests passed - $passed"
echo "tests failed - $failed"
