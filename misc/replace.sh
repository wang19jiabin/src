#!/bin/bash

replace_name()
{
	for old in `find -name "*$1*"`
	do
		new=`echo $old | sed "s/$1/$2/"`
		mv $old $new
	done
}

replace_content()
{
	for file in `grep -rl $1`
	do
		sed -i "s/$1/$2/g" $file
	done
}

if [ $# -ne 2 ]
then
	echo "Usage: $0 old new" >&2
	exit
fi

read -p"Make sure your repo is clean! Input 'yes' to continue: " input
if [ "$input" != 'yes' ]
then
	echo exit >&2
	exit
fi

replace_name $1 $2
replace_content $1 $2
