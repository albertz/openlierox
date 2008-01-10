# Copyright 1999-2004 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

inherit eutils games toolchain-funcs

DESCRIPTION="A real-time excessive Worms-clone"
HOMEPAGE="http://openlierox.sourceforge.net/"
SRC_URI="mirror://sourceforge/openlierox/OpenLieroX_${PV}.src.tar.bz2"

LICENSE="LGPL-2"
SLOT="0"
KEYWORDS="~ppc ~x86 ~amd64"
IUSE="debug"

RDEPEND="media-libs/libsdl
	media-libs/sdl-mixer
	media-libs/sdl-image
	dev-games/hawknl
	media-libs/gd
	sys-libs/zlib
	dev-libs/libxml2"

DEPEND="${RDEPEND}"

src_unpack() {
	mkdir -p ${S} || die "cannot creating working-dir"
	cd ${S}

	unpack OpenLieroX_${PV}.src.tar.bz2 || die "cannot unpack the main archive"
}

src_compile() {
	cd ${S}/OpenLieroX || die "some strange problems ..."

	# SYSTEM_DATA_DIR/OpenLieroX will be the search path
	# the compile.sh will also take care of CXXFLAGS
	SYSTEM_DATA_DIR="${GAMES_DATADIR}" \
	COMPILER=$(tc-getCXX) \
	DEBUG=$(use debug && echo 1 || echo 0) \
	VERSION=${PV} \
	./compile.sh || die "error(s) while compiling; please make a report"
}

src_install() {
	cd ${S}/OpenLieroX || die "some strange problems ..."

	echo ">>> copying binary ..."
	newgamesbin bin/openlierox openlierox || die "cannot copy binary"

	echo ">>> copying gamedata-files ..."
	# HINT: the app uses case-insensitive file-handling
	insinto "${GAMES_DATADIR}"/${PN}/
	doins -r share/gamedir/* || die "failed while copying gamedata"

	echo ">>> installing doc ..."
	dodoc doc/README
	dodoc doc/ChangeLog
	dodoc doc/TODO
	insinto "/usr/share/doc/${PF}"
	doins -r doc/original_lx_docs

	echo ">>> creating icon and desktop entry ..."
	doicon share/OpenLieroX.svg
	make_desktop_entry openlierox OpenLieroX OpenLieroX.svg Game

	prepgamesdirs
}
