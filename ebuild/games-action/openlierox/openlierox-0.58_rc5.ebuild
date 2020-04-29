# Copyright 2020 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

inherit desktop cmake-utils

DESCRIPTION="Liero clone / Worms realtime / 2D shooter"
HOMEPAGE="http://openlierox.net"
SRC_URI="https://github.com/albertz/openlierox/archive/${PV}.tar.gz"

LICENSE="LGPL-2"
SLOT="0"
KEYWORDS="~amd64 ~x86"
IUSE="X breakpad debug"

DEPEND="media-libs/sdl-mixer
		media-libs/sdl-image
		media-libs/gd[png]
		dev-libs/libxml2
		dev-libs/libzip
		sys-libs/zlib
		net-misc/curl
		X? (
				x11-libs/libX11
				media-libs/libsdl[X]
		)
		!X? ( media-libs/libsdl )"
RDEPEND="${DEPEND}"
BDEPEND=""

src_configure() {
	local mycmakeargs=(
		-D DEBUG="$(usex debug)"
		-D X11="$(usex X)"
		-D BREAKPAD="$(usex breakpad)"
		-D SYSTEM_DATA_DIR="${GAMES_DATADIR}"
		-D LIBZIP_BUILTIN="no")

	cmake-utils_src_configure
}

src_compile() {
	cmake-utils_src_compile
}

src_install() {
	# NOTE: App uses case-insensitive file-handling
	# NOTE: /usr/share/games is used by the upstream
	insinto "/usr/share/games/${PN}/"
	doins -r share/gamedir/* || die "doins failed"

	dodoc doc/{README,ChangeLog,Development,TODO} || die "dodoc failed"
	insinto "/usr/share/doc/${PF}"
	doins -r doc/original_lx_docs || die "doins failed"

	doicon share/OpenLieroX.* || die "doicon failed"
	make_desktop_entry openlierox OpenLieroX OpenLieroX \
			"Game;ActionGame;ArcadeGame;" || die "make_desktop_entry failed"

	dobin "${BUILD_DIR}/bin/openlierox" || die "dobin failed"
}
