<html>
<head><title>OpenLieroX</title></head>

<?php
	// read_from_file
	include_once("fileio.php");
?>

<body>
<p>This is the homepage of <b>OpenLieroX</b>.</p>
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
	<img src="pics/lierox3.png"><br><br>
	<img src="pics/lierox31.png">
</p>
<p>
	<h2>Downloads</h2>
	<b>0.57_beta3</b> (released 2007-08-08)<br>
	<a href="http://sourceforge.net/project/showfiles.php?group_id=180059&package_id=208133&release_id=531327">
	OpenLieroX main downloads</a> (with Mac OS X Universal Binary now!)<br>
	<a href="ebuild/games-action/openlierox/openlierox-0.57_beta3.ebuild">OpenLieroX Gentoo ebuild</a><br>
	HINT: We already provide a Deb-package but this package is not conform to all the 
	Debian-policies. If somebody here is familiar with deb-packages, take a look at the
	RFP-bugtracker-entry here: <a href="http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=445850">
	http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=445850</a><br>
	<br>
	<b>0.57_beta2</b> (released 2007-04-09)<br>
	<a href="http://sourceforge.net/project/showfiles.php?group_id=180059&package_id=208133&release_id=499780">
	OpenLieroX downloads</a><br>
	<a href="ebuild/games-action/openlierox/openlierox-0.57_beta2.ebuild">OpenLieroX Gentoo ebuild</a><br>
	<br>
	<b>0.57_beta1</b> (released 2007-01-27)<br>
	<a href="tarball/OpenLieroX_0.57_beta1.src.tar.bz">OpenLieroX Source tar.bz</a><br>	
	<a href="tarball/OpenLieroX_0.57_beta1.src.zip">OpenLieroX Source zip</a><br>
	<a href="ebuild/games-action/openlierox/openlierox-0.57_beta1.ebuild">OpenLieroX Gentoo ebuild</a><br>
	<br>
	<b>Levels and mods</b> (they are now included since Beta2 in the game)<br>
	<a href="additions/lx0.56_pack1.9.zip">LieroX 0.56 Pack 1.9</a><br>
	<a href="additions/another_lx_pack_2007_01_05.zip">another LX Pack (2007-01-05)</a><br>
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
		wget http://openlierox.sourceforge.net/games-action/openlierox/openlierox-0.57_beta3.ebuild
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
	If you like to write any comments, bug reports or anything else, use the following mail-adress for now:<br>
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
	<a href="http://iworx5.webxtra.net/~lxallian/LXRS/">official LieroX site</a><br>
	<a href="http://open.liero.be/">Open Liero</a><br>
	<a href="http://gusanos.sourceforge.net/">Gusanos</a><br>
	<a href="http://vermes.dubdot.com/">Vermes</a>
	<hr>
	<a href="http://www.az2000.de/">Alberts homepage</a><br>
</p>
</body>
</html>

