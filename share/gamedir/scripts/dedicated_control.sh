#!/bin/bash

while read signal; do
	echo "$signal"

	c=0
	while [ $c -le 10 ]; do
		sleep 1
		echo "c = $c"
		c=$(expr $c + 1)
	done
done


