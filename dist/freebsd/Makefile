PORTNAME=	disgrace
DISTVERSION=	0.1.0
CATEGORIES=	audio

MAINTAINER=	miroslav.shaltev@shaltev.de
COMMENT=	Minimalist tracker-style digital audio workstation
WWW=		https://github.com/cvscpp/disgrace

LICENSE=	GPLv3+
LICENSE_FILE=	${WRKSRC}/LICENSE

# Fetch the release tag directly from GitHub (approach 1.B): the
# auto-generated tarball carries no configure script, so it is rebuilt
# below with autoreconf. The AX_CXX_COMPILE_STDCXX macro is vendored in
# m4/, therefore no autoconf-archive dependency is needed.
USE_GITHUB=	yes
GH_ACCOUNT=	cvscpp
GH_TAGNAME=	v${DISTVERSION}

USES=		autoreconf compiler:c++17-lang pkgconfig
USE_WX=		3.2

GNU_CONFIGURE=	yes

LIB_DEPENDS=	libSoundTouch.so:audio/soundtouch \
		libarchive.so:archivers/libarchive \
		libespeak-ng.so:audio/espeak-ng \
		libfftw3.so:math/fftw3 \
		libfluidsynth.so:audio/fluidsynth \
		libjack.so:audio/jack \
		libsamplerate.so:audio/libsamplerate \
		libsndfile.so:audio/libsndfile \
		libxml2.so:textproc/libxml2
BUILD_DEPENDS=	nlohmann-json>0:devel/nlohmann-json

# Festival and Piper TTS support is auto-detected by configure and stays
# disabled unless the corresponding dependencies are present.
# ALSA is optional on FreeBSD; DSSI bridging still builds without it.

PLIST_FILES=	bin/disgrace \
		bin/dssi_sandbox \
		%%DATADIR%%/imgs/disgrace.png

.include <bsd.port.mk>
