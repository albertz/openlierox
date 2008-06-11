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

#echo "getcomputerwormlist"

setvar GameServer.ServerName "- simple dedicated server -"

setvar GameOptions.LastGame.LevelName           "CastleStrike.lxl"
setvar GameServer.GameInfo.sMapFile             "CastleStrike.lxl"
setvar GameServer.GameInfo.sMapName             "CastleStrike"

setvar GameOptions.LastGame.ModName             "MW 1.0"
setvar GameServer.GameInfo.sModDir              "MW 1.0"
setvar GameServer.GameInfo.sModName             "MW 1.0"

setvar GameOptions.LastGame.LoadingTime         "20"
setvar GameServer.GameInfo.iLoadingTimes        "20"

setvar GameServer.WeaponRestrictionsFile        "cfg/presets/Mortar Only.wps"


# ------------- start game here ----

startlobby

while true; do
	until start_with_countdown; do sleep 1; done
done

