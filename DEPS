-- general stuff
Gentoo: gcc
Debian: build-essential

-- libsdl
Gentoo: media-libs/libsdl
Debian: libsdl1.2-dev
tested versions: 1.2.11, 1.2.6

-- SDL_mixer
Gentoo: media-libs/sdl-mixer
Debian: libsdl-mixer1.2-dev
tested versions: 1.2.7

-- SDL_image
Debian: libsdl-image1.2-dev
Gentoo: media-libs/sdl-image
tested versions: 1.2.5

-- hawknl
* builtin version possible
Gentoo: dev-games/hawknl
Debian: no deb-package found, you have to install it manually
tested versions: 1.68

-- gd
Gentoo: media-libs/gd
Debian: libgd2-noxpm-dev
tested versions: 2.0.33

-- zlib
Gentoo: sys-libs/zlib
Debian: zlib1g-dev
tested versions: 1.2.3

-- libzip
* builtin version possible
Gentoo: dev-libs/libzip
Debian: libzip-dev
tested version: 0.8

-- libxml2
Gentoo: dev-libs/libxml2
Debian: libxml2-dev
tested versions: 2.6.26

-- libcurl
Gentoo: net-misc/curl
Debian: libcurl4-dev
tested versions: 7.19.6

----

Quick command for Debian/Ubuntu:
sudo apt-get install build-essential subversion cmake libsdl1.2-dev libsdl-mixer1.2-dev libsdl-image1.2-dev libgd2-noxpm-dev zlib1g-dev libzip-dev libxml2-dev libx11-dev libcurl4-gnutls-dev
cmake -D HAWKNL_BUILTIN=1 -D DEBUG=0 -D X11=1 .

Quick command for OpenBSD:
pkg_add subversion cmake sdl sdl-image sdl-mixer libxml gd

Quick command for FreeBSD/PC-BSD:
pkg_add -r subversion cmake sdl sdl_image sdl_mixer libxml2 gd
cmake -DHAWKNL_BUILTIN=1 -DLIBZIP_BUILTIN=1 -D DEBUG=0 .

