#!/bin/sh

for i in 2 3 4 5; do
	rm /etc/rc$i.d/S92openlierox
done
for i in 0 1 6; do
	rm /etc/rc$i.d/K08openlierox
done

rm /etc/init.d/openlierox
