<html>
<head><title>OpenLieroX</title></head>

<body>
<p>This is the homepage of <b>OpenLieroX</b>.</p>
<p>
	<h2>Description</h2>
	OpenLierox is an extremely addictive realtime worms shoot-em-up backed
	by an active gamers community.<br>
	Dozens of levels and mods are available to provide endless gaming pleasure.
</p>
<p>
	<h2>About</h2>
	The original game was coded by Jason Boettcher and later
	released under the zlib-licence.<br>
	This version is based on it, ported to Linux and a lot enhanced
	by Dark Charlie and Albert Zeyer.
</p>
<p>
	<h2>Screenshots</h2>
	<img src="lierox3.png"><br><br>
	<img src="lierox31.png">
</p>
<p>
	<h2>Downloads</h2>
	<a href="OpenLieroX_0.57.src.zip">OpenLieroX 0.57 Source zip</a><br>
	<a href="OpenLieroX_0.57.src.tar.bz">OpenLieroX 0.57 Source tar.bz</a><br>	
	<a href="OpenLieroX_0.57.win32.zip">OpenLieroX 0.57 Win32 binary zip</a><br>
	<a href="lx0.56_pack1.9.zip">LieroX 0.56 Pack 1.9</a><br>
	<a href="games-action/openlierox/openlierox-0.57.ebuild">Gentoo OpenLieroX ebuild</a>
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
		wget http://openlierox.sourceforge.net/games-action/openlierox/openlierox-0.57.ebuild
		echo "games-action/openlierox ~x86" >> /etc/portage/package.keywords
		FEATURES="-strict" emerge openlierox
	</pre>
</p>
<p>
	<h2>Installation somewhere</h2>
	Download the source and extract it. Take a look into the file
	<i>DEPS</i> for the information, which dependencies are needed.
	Install the missing dependencies.<br>
	Then use the <i>compile.sh</i> to
	compile it. If you want to install it into your system, use the
	<i>install.sh</i>. Take a look at these both scripts, if you want
	information about environment-variables you can use.<br>
	Use the start.sh script, if you don't want to install it.<br>
	For example:<br>
	<pre>
		./compile.sh
		./start.sh
	</pre>
</p>
<p>
	<h2>Installation under Debian/Ubuntu</h2>
	Follow the installation @somewhere. You have only one problem:
	HawkNL doesn't exist for Debian/Ubuntu. But there is the possibility
	to compile OpenLieroX with HawkNL builtin. Simply do:
	<pre>
		HAWKNL_BUILTIN=1 ./compile.sh
	</pre>
</p>
<p>
	<h2>Details</h2>
	The game uses case insensitive filenames (it will use the first 
	found on case sensitive filesystems).<br>
	The game searches the paths ~/.OpenLieroX, 
	./ and /usr/share/OpenLieroX
	(or under Gentoo: /usr/share/games/OpenLieroX)
	for game-data (all path are relativ to this bases) (in this 
	order). You can also
	add more searchpathes in cfg/options.cfg (you also can change 
	the searchpath-order here). Own modified configs,
	screenshots and other stuff always will be stored in 
	~/.OpenLieroX.
</p>
<p>
	<h2>Links</h2>
	<a href="http://sourceforge.net/projects/openlierox/">SourceForge project-site</a><br>
	<a href="http://lxalliance.net/lierox/">official LieroX site</a>
</p>
</body>
</html>
