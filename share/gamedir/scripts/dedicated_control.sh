#!/bin/bash

CURDIR="`dirname $0`"

function startlobby() {
	echo "startlobby"
}

function startgame() {
	echo "startgame"
}

function msg() {
	echo "msg $1"
}

function chatmsg() {
	echo "chatmsg $1"
}

function setvar() {
	echo "setvar $1 $2"
}

function sendlobbyupdate() {
	echo "sendlobbyupdate"
}

function start_with_countdown() {
	c=20
	while [ "$c" -ge 1 ]; do
		[ "$c" -ge 10 ] && d=2 || d=1
		msg "$c ..."
		chatmsg "$c ..."
		sleep $d
		c=$(expr "$c" - $d)
	done

	startgame
	if wait_for_gamestart; then
		wait_for_gameend
		return 0
	fi
	return 1
}

function wait_for_gamestart() {
	local signal
	while read signal; do
		[ "$signal" == "gameloopstart" ] && {
			chatmsg "prepare!"
			return 0
		}
		[ "$signal" == "errorstartgame" ] && return 1
		[ "$signal" == "quit" ] && exit
	done
}

function wait_for_gameend() {
	local signal
	while read signal; do
		msg "gameloop: $signal"
		[ "$signal" == "backtolobby" ] && {
			sleep 1
			chatmsg "back in lobby, waiting ..."
			return
		}
		[ "$signal" == "quit" ] && exit
	done
}

function signal_handler() {
	local signal
	while read signal; do
		msg "lobby: $signal"
		[ "$signal" == "quit" ] && exit
	done
}

preset=0

function select_next_preset() {
	p=0
	for f in $CURDIR/presets/* ; do
		if [ $p == $preset ] ; then
			msg "Preset `basename $f`"
			chatmsg "Preset `basename $f`"
			. "$f"
		fi
		p=`expr $p '+' 1`
	done
	preset=`expr $preset '+' 1`
	if [ $preset -ge $p ]; then
		preset=0
	fi
	sendlobbyupdate
}

echo "getcomputerwormlist"

setvar GameServer.ServerName "Dedicated multimod"

startlobby

while true; do
	select_next_preset
	until start_with_countdown; do sleep 1; done
done
