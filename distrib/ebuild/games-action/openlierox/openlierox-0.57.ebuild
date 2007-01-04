# Copyright 1999-2004 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

inherit eutils games toolchain

DESCRIPTION="OpenLieroX is a real-time excessive Worms-clone"
HOMEPAGE="http://sourceforge.net/projects/openlierox/"
SRC_URI="http://openlierox.sourceforge.net/OpenLieroX_${PV}.src.zip
	http://openlierox.sourceforge.net/lx0.56_pack1.9.zip"

LICENSE="LGPL-2"
SLOT="0"
KEYWORDS="~amd64 ~ppc ~x86"
IUSE=""

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

	unpack OpenLieroX_${PV}.src.zip || die "cannot unpack the main archive"
	mkdir -p ${S}/share/gamedir/packtmp && \
	cd ${S}/share/gamedir/packtmp && \
	unpack lx0.56_pack1.9.zip && \
	mv -f * .. && cd .. && rmdir packtmp \
		|| die "cannot unpack the LieroX Pack"
}

src_compile() {
	cd ${S} || die "some strange problems ..."

	# SYSTEM_DATA_DIR/OpenLieroX will be the search path
	# the compile.sh will also use the CXXFLAGS
	SYSTEM_DATA_DIR="${GAMES_DATADIR}" \
	COMPILER=$(tc-getCXX) \
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
	insinto "/usr/share/doc/${PF}"
	doins -r doc/original_lx_docs

	echo ">>> creating icon and desktop entry ..."
	doicon share/OpenLieroX.png
	make_desktop_entry openlierox OpenLieroX OpenLieroX.png Game

	prepgamesdirs
}
