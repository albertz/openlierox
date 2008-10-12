#!/bin/bash

CURDIR="`dirname $0`"

function startlobby() {
	echo "startlobby"
}

function addbot() {
	echo "addbot"
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


# ------------- start game here ----

startlobby

setvar GameServer.ServerName "* OLX Beta8 dedicated server *"

setvar GameServer.GameInfo.sMapFile             "CastleStrike.lxl"
setvar GameServer.GameInfo.sMapName             "CastleStrike"

setvar GameServer.GameInfo.sModDir              "MW 1.0"
setvar GameServer.GameInfo.sModName             "MW 1.0"

setvar GameServer.GameInfo.iLoadingTimes        0
setvar GameServer.GameInfo.fGameSpeed			1.3
setvar GameServer.GameInfo.iLives				-2
setvar GameServer.GameInfo.fTimeLimit			10
setvar GameServer.GameInfo.bAllowConnectDuringGame	true
setvar GameServer.GameInfo.bForceRandomWeapons		true
setvar GameServer.GameInfo.bSameWeaponsAsHostWorm	true

setvar GameServer.WeaponRestrictionsFile        "cfg/presets/Mortar Only.wps"

addbot

while true; do
	until start_with_countdown; do sleep 1; done
done

