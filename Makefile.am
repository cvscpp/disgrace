ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src 

pkgdataimgdir = $(pkgdatadir)/imgs
pkgdataimg_DATA = imgs/disgrace.png

EXTRA_DIST = \
    $(pkgdataimg_DATA) \
    README.md \
    AUTHORS \
    LICENSE \
    NEWS \
    dist/freebsd/Makefile \
    dist/freebsd/distinfo \
    dist/freebsd/pkg-descr \
    dist/freebsd/README \
    dist/debian/control \
    dist/debian/rules \
    dist/debian/changelog \
    dist/debian/copyright \
    dist/debian/source/format \
    dist/debian/disgrace.1 \
    dist/debian/README \
    dist/gentoo/metadata/layout.conf \
    dist/gentoo/media-sound/disgrace/disgrace-0.1.2.ebuild \
    dist/gentoo/media-sound/disgrace/metadata.xml \
    dist/gentoo/README

# Maximum-speed local build: -O3, link-time optimization, and tuning for
# this CPU family (-march=native -mtune=native). The resulting binary is
# NOT portable to other CPUs — never use it for distro packaging or
# release tarballs (those stay generic: -O3 without -march).
# Reconfigures + full rebuild, so it takes a while. Example:
#   make production -j8
.PHONY: production
production:
	@echo "=== Production build: -O3 -flto -march=native ==="
	@echo "=== NOTE: binary will only run on this CPU family ==="
	$(MAKE) $(AM_MAKEFLAGS) clean
	./configure --enable-lto --enable-native-march
	$(MAKE) $(AM_MAKEFLAGS)
