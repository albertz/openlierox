# Copyright 1999-2004 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

inherit eutils games toolchain-funcs

DESCRIPTION="OpenLieroX is a real-time excessive Worms-clone"
HOMEPAGE="http://openlierox.sourceforge.net/"
SRC_URI="
	mirror://sourceforge/openlierox/OpenLieroX_${PV}.src.tar.bz
	mirror://sourceforge/openlierox/lx0.56_pack1.9.zip
	mirror://sourceforge/openlierox/another_lx_pack_2007_01_05.zip
	http://openlierox.sourceforge.net/OpenLieroX_${PV}.src.tar.bz
	http://openlierox.sourceforge.net/lx0.56_pack1.9.zip
	http://openlierox.sourceforge.net/another_lx_pack_2007_01_05.zip"

LICENSE="LGPL-2"
SLOT="0"
KEYWORDS="~ppc ~x86"
IUSE="debug"

RDEPEND="media-libs/libsdl
	media-libs/sdl-mixer
	media-libs/sdl-image
	dev-games/hawknl
	media-libs/gd
	sys-libs/zlib
	dev-libs/libxml2"

DEPEND="${RDEPEND}
	app-arch/unzip"

src_unpack() {
	mkdir -p ${S} || die "cannot creating working-dir"
	cd ${S}

	unpack OpenLieroX_${PV}.src.tar.bz || die "cannot unpack the main archive"
	mkdir -p ${S}/share/gamedir/packtmp && \
	cd ${S}/share/gamedir/packtmp && \
	unpack lx0.56_pack1.9.zip && \
	unpack another_lx_pack_2007_01_05.zip && \
	cp -a * .. && cd .. && rm -rf packtmp \
		|| die "cannot unpack the LieroX Pack"
}

src_compile() {
	cd ${S} || die "some strange problems ..."

	# SYSTEM_DATA_DIR/OpenLieroX will be the search path
	# the compile.sh will also take care of CXXFLAGS
	SYSTEM_DATA_DIR="${GAMES_DATADIR}" \
	COMPILER=$(tc-getCXX) \
	DEBUG=$(use debug && echo 1 || echo 0) \
	./compile.sh
}

src_install() {
	echo ">>> copying binary ..."
	newgamesbin bin/openlierox openlierox || die "cannot copy binary"

	echo ">>> copying gamedata-files ..."
	insinto "${GAMES_DATADIR}"/${PN}/
	doins -r share/gamedir/* || die "failed while copying gamedata"

	echo ">>> installing doc ..."
	dodoc doc/README
	dodoc doc/ChangeLog
	dodoc doc/TODO
	insinto "/usr/share/doc/${PF}"
	doins -r doc/original_lx_docs

	echo ">>> creating icon and desktop entry ..."
	doicon share/OpenLieroX.png
	make_desktop_entry openlierox OpenLieroX OpenLieroX.png Game

	prepgamesdirs
}
