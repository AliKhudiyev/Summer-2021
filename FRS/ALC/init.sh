#!/bin/sh

if [ -d $1 ]
then
	printf "System directory '%s' already exists, overwrite? [y/n] " "$1"
	read ans
	if [ "$ans" == "n" ]
	then
		exit 1
	fi
	rm -r $1
fi

mkdir -p $1/{in,out}
exit 0
