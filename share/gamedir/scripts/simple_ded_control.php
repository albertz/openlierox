#/usr/bin/php
<?php

$stdin = fopen('php://stdin', 'r');

function startlobby() {
	echo "startlobby\n"; 
	flush();
}

function startgame() {
	echo "startgame\n";
	flush();
}

function msg($s) {
	echo "msg " . $s . "\n";
	flush();
}

function chatmsg($s) {
	echo "chatmsg " . $s . "\n";
	flush();
}

function setvar($var, $value) {
	echo "setvar " . $var . " " . $value . "\n";
	flush();
}

function sendlobbyupdate() {
	echo "sendlobbyupdate\n";
	flush();
}

function start_with_countdown() {
	$c=20;
	while ( $c > 1 ) {
		if( $c > 10 ) { $d=2; } else { $d=1; };
		msg( $c . " ..." );
		chatmsg( $c . " ..." );
		sleep( $d );
		$c = $c - $d;
	}

	startgame();
	if( wait_for_gamestart() ) {
		wait_for_gameend();
		return 1;
	}
	return 0;
}

function wait_for_gamestart() {
	global $stdin;
	while($signal = fgets($stdin, 4096)) {
		$signal = trim($signal);
		if( $signal == "gameloopstart" ) {
			chatmsg( "prepare!" );
			return 1;
		}
		if( $signal == "errorstartgame" ) { return 0; }
		if( $signal == "quit" ) { exit(); }
	}
}

function wait_for_gameend() {
	global $stdin;
	while($signal = fgets($stdin, 4096)) {
		$signal = trim($signal);
		msg( "gameloop: " . $signal );
		if( $signal == "backtolobby" ) {
			sleep( 1 );
			chatmsg( "back in lobby, waiting ..." );
			return 1;
		}
		if( $signal == "quit" ) { exit(); }
	}
}

function signal_handler() {
	global $stdin;
	while($signal = fgets($stdin, 4096)) {
		$signal = trim($signal);
		msg( "lobby: " . $signal );
		if( $signal == "quit" ) { exit(); }
	}
}

# ------------- start game here ----

startlobby();

setvar( "GameServer.ServerName", "- simple dedicated server -");

setvar( "GameOptions.LastGame.LevelName",           "CastleStrike.lxl" );
setvar( "GameServer.GameInfo.sMapFile",             "CastleStrike.lxl" );
setvar( "GameServer.GameInfo.sMapName",             "CastleStrike" );

setvar( "GameOptions.LastGame.ModName",             "MW 1.0" );
setvar( "GameServer.GameInfo.sModDir",              "MW 1.0" );
setvar( "GameServer.GameInfo.sModName",             "MW 1.0" );

setvar( "GameOptions.LastGame.LoadingTime",         "20" );
setvar( "GameServer.GameInfo.iLoadingTimes",        "20" );

setvar( "GameServer.WeaponRestrictionsFile",        "cfg/presets/Mortar Only.wps" );

while( 1 ) {
	start_with_countdown(); 
	sleep( 1 );
}

?>

