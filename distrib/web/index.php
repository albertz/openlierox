<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>
<head>
<title>OpenLieroX</title>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8"/>
<meta name="author" content="Albert Zeyer, Karel Petránek, Martin Griffin, Pelya">
<meta name="keywords" content="OpenLieroX,Liero,LieroX,LX,Worms,2D-Quake,Linux,Windows,MacOSX,Mac OS X,Mac,Albert Zeyer,Karel Petránek">
<meta name="description" content="OpenLieroX - extremely addictive realtime worms shoot-em-up backed by an active gamers community - like Liero">
</head>

<?php
	// read_from_file
	include_once("fileio.php");
?>

<body>
<table width="100%" border="0" vspace="0" hspace="0">
<tr><td>
<img src="pics/olx.png" align="left">
<h1><font color="green">This is the homepage of <i>OpenLieroX</i>.</font></h1>
</td></tr></table>
<hr height="1">
<p>
	<h2>Short Description</h2>
	OpenLierox is an extremely addictive realtime worms shoot-em-up backed by an active gamers community.<br>
	Dozens of levels and mods are available to provide endless gaming pleasure.
</p>
<p>
	<h2>Long Description</h2>
	OpenLieroX is based and compatible to the famous LieroX. LieroX is a 2D shooter game. It is an unofficial sequel to Liero, and is the most popular of all the Liero clones. It features online play, fully customizable weapons, levels and characters. Liero Xtreme was created in C++ by Jason 'JasonB' Boettcher, an Australian programmer.<br>
	<br>
	The game is based on a deathmatch setting, where multiple players face off in a closed level. Each player is equipped with five weapons selected out of all the weapons allowed, and with a ninja rope that allows the player to move in any direction. Players begin with a set amount of lives, and whilst the game records the number of kills, the last man standing is usually considered the winner. LieroX also allows team deathmatches, which has made it common for players to form clans.<br>
	<br>
	Because of the huge community, there are dozens of levels and mods available. You also have no problem to find somebody on Internet to play with. Or if you want to play offline, you also can play with bots.
</p>
<p>
	<h2>About</h2>
	The original game was coded by Jason Boettcher and later released under the zlib-licence.<br>
	This version is based on it, ported to Linux and Mac OS X and a lot enhanced by Karel Petránek and Albert Zeyer.
</p>
<p>
	<h2>Screenshots</h2>
	<img src="pics/Bild%2017.png"><br><br>
	<img src="pics/lierox3.png"><br><br>
	<img src="pics/lierox31.png">
</p>
<p>
	<h2>Downloads</h2>
	<b>0.57_beta5</b> (released 2008-03-15)<br>
	<a href="http://sourceforge.net/project/showfiles.php?group_id=180059&package_id=208133&release_id=584470">
	OpenLieroX main downloads</a> (Source/Linux, Windows, Mac OS X)<br>
	<a href="ebuild/games-action/openlierox/openlierox-0.57_beta5.ebuild">OpenLieroX Gentoo ebuild</a><br>
<pre>
0.57_beta5
==========
- fast and easy theme switching
  ( and LieroX 0.56 theme included )
- disallow strafing on server if wanted
- optimised/fixed joystick support
- optimised network
- fixed aiming bug of Beta4
- spectate option for host
- bugfixes
</pre>
	<b>0.57_beta4</b> (released 2008-02-23)<br>
	<a href="http://sourceforge.net/project/showfiles.php?group_id=180059&package_id=208133&release_id=578993">
	OpenLieroX main downloads</a> (Source/Linux, Windows, Mac OS X)<br>
	<a href="ebuild/games-action/openlierox/openlierox-0.57_beta4.ebuild">OpenLieroX Gentoo ebuild</a><br>
<pre>
0.57_beta4
==========
- very basic dedicated server
- joystick support (for playing)
- mouse support (for playing)
- copy&paste support for Linux and MacOSX
- config file got parsed completly and unknown options
  are kept, therefore forward-compatible
- better forward-compatibility for net-protocol
- FPS independent physics
- more correct projectile simulation
- experimental file transfer support
- delete-key works now under MacOSX
- more keys got recognised
  (super and meta keys, that includes also the
  Apple-key on a Mac)
- support for multiple masterservers
- more intelligent event-handling (in some parts);
  all pressed keys got recognised in chat/console
  and you can also play on very low FPS
- DNS support for serverlist
- Worm AI improvements
- usage of ALSA on Linux by default
- network improved
- cache for map and gamescript
- Developer documentation
- strafing
- optimised sorting in combobox
- FPS limit is also valid for menu
- menu takes less resources
- help for weapon selection
- fixed changing of graphic settings
- and a lot of other fixes / cleanups
</pre>
	<b>0.57_beta3</b> (released 2007-08-08)<br>
	<a href="http://sourceforge.net/project/showfiles.php?group_id=180059&package_id=208133&release_id=531327">
	OpenLieroX main downloads</a> (with Mac OS X Universal Binary now!)<br>
	<a href="ebuild/games-action/openlierox/openlierox-0.57_beta3.ebuild">OpenLieroX Gentoo ebuild</a><br>
<pre>
0.57_beta3
==========
- 16/24/32 bit graphic support and OpenGL support
- runs on MacOSX 
- new design & icon (thanks goes to Raziel)
- probably fixed any saving-problem
  (problem was: sometimes SDL_Quit gives a segfault
  and saving was done after this; now it's done before)
- full Unicode/UTF8 support
- font antialasing support
- working Dev-C++ project (therefore MingW support)
- MaxFPS in option-dialog
- Worm AI improvements
- ninjarope physic simulation is FPS independent
- profile-saving is correct now
</pre>
	<b>0.57_beta2</b> (released 2007-04-09)<br>
	<a href="http://sourceforge.net/project/showfiles.php?group_id=180059&package_id=208133&release_id=499780">
	OpenLieroX downloads</a><br>
	<a href="ebuild/games-action/openlierox/openlierox-0.57_beta2.ebuild">OpenLieroX Gentoo ebuild</a><br>
<pre>
0.57_beta2
==========
- BPP independent
- ingame mediaplayer
- 64bit version works now
- more fixes, general improvements, etc.
- code cleanups (replacements with C++/STL technics)
- whole project uses std::string now (realy huge change)
- collision-checks are improved (more like the original LX, that
  means, it goes at least through walls with ~3px width)
- chatbox improvements
- unlimited number of bots in net play
- ~/ at the beginning of a searchpath is handled correctly and there
  is a more correct method to get the homedir
- MaxFPS option under [Advanced] in ~/.OpenLieroX/cfg/options.cfg
- fixed compile-issues on 64bit systems
- no-clipping-issues with weapon-generated dirt are fixed
</pre>
	<b>0.57_beta1</b> (released 2007-01-27)<br>
	<a href="tarball/OpenLieroX_0.57_beta1.src.tar.bz">OpenLieroX Source tar.bz</a><br>	
	<a href="tarball/OpenLieroX_0.57_beta1.src.zip">OpenLieroX Source zip</a><br>
	<a href="ebuild/games-action/openlierox/openlierox-0.57_beta1.ebuild">OpenLieroX Gentoo ebuild</a><br>
<pre>
0.57_beta1
==========
- POSIX-compatible (well-tested under Linux).
- endian-independent (runs without any problems on my big endian PPC)
- a _huge_ amount of bug-fixes
- the old BASS soundsystem was replaced by SDL_mixer
- new file handling routines
  (1. based on searchpaths; 2. case-insensitive handling)
- new bot AI with a very nice pathfinding-algo
- some other additions/extensions/...
- ...
</pre>
	<b>additional things</b><br>
	<a href="additions/original_lxfrontend_for_beta4.zip">original LieroX frontend for Beta4</a>
	(just extract it to your home-searchpath, that is <i>~/.OpenLieroX</i> under Linux, <i>Documents/OpenLieroX</i> under Windows, <i>~/Library/Application Support/OpenLieroX</i> under MacOSX)
</p>
<p>
	<h2>Installation under Gentoo</h2>
	Download the provided ebuild and install it.<br>
	For example, you can do this by (bad but simple way):<br>
	<pre>
		su -
		cd /usr/portage
		mkdir -p games-action/openlierox
		cd games-action/openlierox
		wget http://openlierox.sourceforge.net/games-action/openlierox/openlierox-0.57_beta4.ebuild
		echo "games-action/openlierox ~x86" >> /etc/portage/package.keywords
		FEATURES="-strict" emerge openlierox
	</pre>
	(Feel free to post any success-stories on Gentoo
	at <a href="http://bugs.gentoo.org/show_bug.cgi?id=164009">this topic</a>,
	related to the ebuild, on the Gentoo-Bugtracker.)
</p>
<p>
	<h2>Installation somewhere</h2>
	Download the source and extract it. Take a look into the file <i>DEPS</i> for the information, which dependencies are needed. Install the missing dependencies.<br>
	Then use the <i>compile.sh</i> to compile it. If you want to install it into your system, use the <i>install.sh</i>. Take a look at these both scripts, if you want information about environment-variables you can use.<br>
	Use the start.sh script, if you don't want to install it.<br>
	For example:<br>
	<pre>
		./compile.sh
		./start.sh
	</pre>
</p>
<p>
	<h2>Installation under Debian/Ubuntu</h2>
	There is a deb-package of Beta3 for you. This package currently doesn't do the dependency check for you, so you have still to install all needed dependencies (libraries) manually.<br>
	The following commands should install all needed dependencies on your system:
	<pre>
		sudo apt-get install build-essential
		sudo apt-get install libsdl1.2-dev 
		sudo apt-get install libsdl-mixer1.2-dev
		sudo apt-get install libsdl-image1.2-dev 
		sudo apt-get install libgd2-noxpm-dev
		sudo apt-get install zlib1g-dev
		sudo apt-get install libxml2-dev
	</pre>
	If you want to compile/install it manually yourself:<br>
	Follow the installation @somewhere. You have only one problem: HawkNL doesn't exist for Debian/Ubuntu. But there is the possibility to compile OpenLieroX with HawkNL builtin. Simply do (after you have installed the needed dependencies):
	<pre>
		HAWKNL_BUILTIN=1 ./compile.sh
	</pre>
</p>
<p>
	<h2>Details</h2>
	The game uses case insensitive filenames (it will use the first found on case sensitive filesystems).<br>
	Under Linux, the game searches the paths <i>~/.OpenLieroX</i>, <i>./</i> and <i>/usr/share/OpenLieroX</i> (or under Gentoo: <i>/usr/share/games/OpenLieroX</i>) for game-data (all path are relativ to this bases) (in this order). You can also add more searchpathes in <i>cfg/options.cfg</i> (you also can change the searchpath-order here). Own modified configs, screenshots and other stuff always will be stored in <i>~/.OpenLieroX</i>.<br>
	Look also into the files in the directory <i>doc/</i>, they may be usefull (also for developers).
</p>
<p>
	<h2>Contact</h2>
	Use the bug-tracker if you want to report a bug. Use the feature request
	tracker if you miss something in the game. Use the forum at LXA or SF
	to meet the community and ask general questions. Register on the
	mailing list if you are interested in the internal development.<br>
	For other things, just write a mail to:
	<i>openlierox [at] az2000 [dot] de</i>
</p>
<p>
	<h2>Links</h2>
	<a href="http://sourceforge.net/projects/openlierox/">SourceForge project-site</a><br>
	<a href="http://sourceforge.net/tracker/?group_id=180059&atid=891648">Bug tracker</a> -
	<a href="http://sourceforge.net/tracker/?group_id=180059&atid=891651">Feature request tracker</a> -
	<a href="http://sourceforge.net/forum/?group_id=180059">Forum at SF</a><br>
	<a href="http://sourceforge.net/mail/?group_id=180059">Mailing lists at SF</a><br>
	<a href="http://lxalliance.net/forum/index.php?board=125.0">LXA forum for OpenLieroX</a><br>
	<hr>
	<a href="http://iworx5.webxtra.net/~lxallian/filebase/file.php?fball=all">FileBase with levels, mods and skins</a><br>
	<a href="http://iworx5.webxtra.net/~lxallian/LXRS/">official LieroX site</a><br>
	<a href="http://open.liero.be/">Open Liero</a><br>
	<a href="http://gusanos.sourceforge.net/">Gusanos</a><br>
	<a href="http://vermes.dubdot.com/">Vermes</a>
	<hr>
	<a href="http://www.az2000.de/">Alberts homepage</a><br>
</p>
</body>
</html>

