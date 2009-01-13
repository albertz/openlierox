#!/bin/sh

cp src/openlierox /etc/init.d
for i in 2 3 4 5; do
	ln -s /etc/init.d/openlierox /etc/rc$i.d/S92openlierox
done
for i in 0 1 6; do
	ln -s /etc/init.d/openlierox /etc/rc$i.d/K08openlierox
done
