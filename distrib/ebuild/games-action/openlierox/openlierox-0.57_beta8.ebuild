# Copyright 1999-2009 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

EAPI=2

inherit eutils games toolchain-funcs

DESCRIPTION="A real-time excessive Worms-clone"
HOMEPAGE="http://openlierox.sourceforge.net/"
SRC_URI="mirror://sourceforge/openlierox/OpenLieroX_${PV}.src.tar.bz2"

LICENSE="LGPL-2"
SLOT="0"
KEYWORDS="~ppc ~x86 ~amd64"
IUSE="X debug"

RDEPEND="media-libs/libsdl
	media-libs/sdl-mixer
	media-libs/sdl-image
	dev-games/hawknl
	media-libs/gd
	dev-libs/libxml2
	dev-libs/libzip
	X? ( x11-libs/libX11
		media-libs/libsdl[X] )"

DEPEND="${RDEPEND}"

MY_PN="OpenLieroX"
MY_P="${MY_PN}_${PV}"
S="${WORKDIR}/${MY_PN}"

src_compile() {
	# SYSTEM_DATA_DIR/OpenLieroX will be the search path
	# the compile.sh will also take care of CXXFLAGS
	SYSTEM_DATA_DIR="${GAMES_DATADIR}" \
	COMPILER=$(tc-getCXX) \
	DEBUG=$(use debug && echo 1 || echo 0) \
	X11=$(use X && echo 1 || echo 0) \
	VERSION=${PV} \
	./compile.sh || die "compilation failed"
}

src_install() {
	einfo "copying binary ..."
	dogamesbin bin/openlierox || die "cannot copy binary"

	einfo "copying gamedata-files ..."
	# HINT: the app uses case-insensitive file-handling
	insinto "${GAMES_DATADIR}"/${PN}/
	doins -r share/gamedir/* || die "failed while copying gamedata"

	einfo "installing doc ..."
	dodoc doc/README doc/ChangeLog doc/Development doc/TODO || die "dodoc failed"
	insinto "/usr/share/doc/${PF}"
	doins -r doc/original_lx_docs || die "doins failed"

	einfo "creating icon and desktop entry ..."
	doicon share/OpenLieroX.* || die "doicon failed"
	make_desktop_entry openlierox OpenLieroX OpenLieroX.svg "Game;ActionGame;ArcadeGame;"

	prepgamesdirs
}
