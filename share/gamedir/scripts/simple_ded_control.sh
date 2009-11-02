#!/bin/bash

CURDIR="$(dirname "$0")"

function waitreturn() {
	local ret
	while read ret; do
		[ "$ret" == "." ] && return
	done
}

function nextsignal() {
	echo "nextsignal"
	local ret
	while read ret; do
		[ "${ret:0:1}" == ":" ] && break || \
		echo "ERROR: wrong input format: $ret" >/dev/stderr
	done
	waitreturn
	signal="${ret:1}"
}

function startlobby() {
	echo "startlobby"
	waitreturn
}

function addbot() {
	echo "addbot"
	waitreturn
}

function startgame() {
	echo "startgame"
	waitreturn
}

function msg() {
	echo "msg $1" || exit -1
	waitreturn
}

function chatmsg() {
	echo "chatmsg $1" || exit -1
	waitreturn
}

function setvar() {
	echo "setvar \"$1\" \"$2\""
	waitreturn
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
	while nextsignal; do
		[ "$signal" == "gameloopstart" ] && {
			chatmsg "prepare!"
			return 0
		}
		[ "$signal" == "errorstartgame" ] && return 1
		[ "$signal" == "quit" ] && exit
		[ "$signal" == "timer" ] && continue # ignore
		msg "wait_for_gamestart: $signal"
	done
}

function wait_for_gameend() {
	while nextsignal; do
		[ "$signal" == "timer" ] && continue # ignore
		msg "gameloop: $signal"
		[ "$signal" == "backtolobby" ] && {
			chatmsg "back in lobby, waiting ..."
			sleep 1
			return
		}
		[ "$signal" == "quit" ] && exit
	done
}

function signal_handler() {
	while nextsignal; do
		msg "lobby: $signal"
		[ "$signal" == "quit" ] && exit
	done
}


# ------------- start game here ----

startlobby

#setvar GameOptions.Network.ServerName "* OLX Beta9 dedicated server *"

setvar GameOptions.GameInfo.LevelName             "CastleStrike.lxl"

setvar GameOptions.GameInfo.ModName             "MW 1.0"

#setvar GameOptions.GameInfo.LoadingTime        0
#setvar GameOptions.GameInfo.GameSpeed			1.3
#setvar GameOptions.GameInfo.Lives			-2
#setvar GameOptions.GameInfo.TimeLimit			5
#setvar GameOptions.GameInfo.AllowConnectDuringGame	true
#setvar GameOptions.GameInfo.ForceRandomWeapons		true
#setvar GameOptions.GameInfo.SameWeaponsAsHostWorm	true

setvar GameServer.WeaponRestrictionsFile        "cfg/presets/Mortar Only.wps"

addbot

while true; do
	until start_with_countdown; do sleep 1; done
done

