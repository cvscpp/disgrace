# Copyright 2026 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=8

WX_GTK_VER="3.2-gtk3"

inherit autotools wxwidgets

DESCRIPTION="Minimalist tracker-style digital audio workstation"
HOMEPAGE="https://github.com/cvscpp/disgrace"
SRC_URI="https://github.com/cvscpp/disgrace/archive/refs/tags/v${PV}.tar.gz -> ${P}.tar.gz"

LICENSE="GPL-3+"
SLOT="0"
KEYWORDS="~amd64"

DEPEND="
	virtual/jack
	x11-libs/wxGTK:${WX_GTK_VER}
	media-libs/libsndfile
	media-libs/libsamplerate
	app-accessibility/espeak-ng
	sci-libs/fftw:3.0
	app-arch/libarchive
	media-sound/fluidsynth
	dev-libs/libxml2
	media-libs/soundtouch
	dev-cpp/nlohmann_json
	media-libs/alsa-lib
"
RDEPEND="${DEPEND}"
BDEPEND="
	virtual/pkgconfig
	sys-devel/autoconf
	sys-devel/automake
"

# The release tarball from `make dist` already ships a configure script,
# but the GitHub tag tarball used here does not, so regenerate it.
src_prepare() {
	default
	eautoreconf
}

# Festival and Piper TTS support auto-detects at configure time and stays
# off unless the corresponding libraries are installed.
