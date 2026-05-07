AC_PREREQ([2.69])
AC_INIT([disgrace], [0.1.0], [miroslav.shaltev@shaltev.de])
AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_HEADERS([config.h])

# Detect host OS for platform-specific configuration
AC_CANONICAL_HOST

# On FreeBSD, GCC mishandles wxWidgets' wchar_t/mbstate conversions at
# runtime.  Clang is the system compiler and must be used instead.
case "$host_os" in
    freebsd*)
        if test -z "$CXX"; then
            AC_CHECK_PROGS([CXX], [clang++ g++])
        fi
        if test -z "$CC"; then
            AC_CHECK_PROGS([CC], [clang gcc])
        fi
        ;;
esac

AM_INIT_AUTOMAKE([foreign subdir-objects -Wall -Werror])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET

# Require C++17
AX_CXX_COMPILE_STDCXX([17], [noext], [mandatory])

# Enable debug option
AC_ARG_ENABLE([debug],
    AS_HELP_STRING([--enable-debug], [Enable debug build]),
    [debug_build=yes],
    [debug_build=no]
)

if test "x$debug_build" = "xyes"; then
    CXXFLAGS="$CXXFLAGS -g -O0 -DDEBUG"
else
    CXXFLAGS="$CXXFLAGS -O2"
fi

# pkg-config
PKG_PROG_PKG_CONFIG

# nlohmann/json
PKG_CHECK_MODULES([NLOHMANN_JSON], [nlohmann_json])

# JACK
PKG_CHECK_MODULES([JACK], [jack >= 1.9.0])

# wxWidgets — the config helper is named differently on FreeBSD
# (e.g. wxgtk3u-3.2-config) vs Linux (wx-config).
AC_CHECK_PROGS([WX_CONFIG], [wx-config wxgtk3u-3.2-config wxgtk3u-config])
if test -n "$WX_CONFIG"; then
    WXWIDGETS_CFLAGS=`$WX_CONFIG --cxxflags`
    WXWIDGETS_LIBS=`$WX_CONFIG --libs`
    AC_SUBST([WXWIDGETS_CFLAGS])
    AC_SUBST([WXWIDGETS_LIBS])
else
    AC_MSG_ERROR([wxWidgets not found. Please install wxWidgets or ensure wx-config / wxgtk3u-3.2-config is in your PATH.])
fi

# libsndfile
PKG_CHECK_MODULES([SNDFILE], [sndfile])

# libsamplerate
PKG_CHECK_MODULES([SAMPLERATE], [samplerate])

# espeak-ng
PKG_CHECK_MODULES([ESPEAK], [espeak-ng])

# Festival — header lives under /usr/include/speech_tools on Linux and
# /usr/local/include/speech_tools on FreeBSD.
AC_LANG_PUSH([C++])
saved_CPPFLAGS="$CPPFLAGS"
FESTIVAL_INCLUDE_DIR=""
for dir in /usr/include/speech_tools /usr/local/include/speech_tools; do
    if test -f "$dir/festival/festival.h"; then
        FESTIVAL_INCLUDE_DIR="$dir"
        break
    fi
done
if test -n "$FESTIVAL_INCLUDE_DIR"; then
    CPPFLAGS="$CPPFLAGS -I$FESTIVAL_INCLUDE_DIR"
    AC_CHECK_HEADER([festival/festival.h], [
        AC_CHECK_LIB([estbase], [main], [
            FESTIVAL_LIBS="-lFestival -lestools -lestbase -leststring -lncurses"
            FESTIVAL_CFLAGS="-I$FESTIVAL_INCLUDE_DIR"
            AC_SUBST([FESTIVAL_LIBS])
            AC_SUBST([FESTIVAL_CFLAGS])
            AC_DEFINE([HAVE_FESTIVAL], [1], [Define if festival is available])
        ], [], [-lestools -leststring -lncurses])
    ], [], [
#include <cstdio>
#include <fstream>
])
fi
CPPFLAGS="$saved_CPPFLAGS"
AC_LANG_POP([C++])

# Piper TTS (neural TTS) — optional, requires libpiper + libonnxruntime.
# Try pkg-config first, then fall back to manual header + library search.
AC_LANG_PUSH([C++])
saved_CPPFLAGS="$CPPFLAGS"
saved_LIBS="$LIBS"
PKG_CHECK_MODULES([PIPER], [piper], [
    AC_DEFINE([HAVE_PIPER], [1], [Define if piper TTS is available])
], [
    # pkg-config not available for piper — search common install prefixes
    PIPER_INCLUDE_DIR=""
    for dir in /usr/include /usr/local/include /opt/piper/include; do
        if test -f "$dir/piper/piper.hpp"; then
            PIPER_INCLUDE_DIR="$dir"
            break
        fi
    done
    if test -n "$PIPER_INCLUDE_DIR"; then
        CPPFLAGS="$CPPFLAGS -I$PIPER_INCLUDE_DIR"
        AC_CHECK_HEADER([piper/piper.hpp], [
            AC_CHECK_LIB([piper], [main], [
                PIPER_CFLAGS="-I$PIPER_INCLUDE_DIR"
                PIPER_LIBS="-lpiper -lonnxruntime"
                AC_SUBST([PIPER_CFLAGS])
                AC_SUBST([PIPER_LIBS])
                AC_DEFINE([HAVE_PIPER], [1], [Define if piper TTS is available])
            ], [AC_MSG_WARN([piper header found but libpiper not found — Piper TTS disabled])],
            [-lonnxruntime])
        ])
    fi
])
CPPFLAGS="$saved_CPPFLAGS"
LIBS="$saved_LIBS"
AC_LANG_POP([C++])

# FFTW3
PKG_CHECK_MODULES([FFTW3], [fftw3])

# libarchive
PKG_CHECK_MODULES([LIBARCHIVE], [libarchive])

# ALSA — required on Linux, optional on FreeBSD (provided by the alsa-lib port)
case "$host_os" in
    freebsd*)
        PKG_CHECK_MODULES([ALSA], [alsa], [],
            [AC_MSG_WARN([ALSA not found on FreeBSD — DSSI support may be limited])])
        ;;
    *)
        PKG_CHECK_MODULES([ALSA], [alsa])
        ;;
esac

# FluidSynth
PKG_CHECK_MODULES([FLUIDSYNTH], [fluidsynth])

# LibXML2
PKG_CHECK_MODULES([LIBXML2], [libxml-2.0])

# Optional: libsndtouch
AC_ARG_ENABLE([sndtouch],
    AS_HELP_STRING([--enable-sndtouch], [Enable time-stretch support]),
    [enable_sndtouch=yes],
    [enable_sndtouch=yes]
)

if test "x$enable_sndtouch" = "xyes"; then
    PKG_CHECK_MODULES([SNDTOUCH], [soundtouch])
    AC_DEFINE([HAVE_SNDTOUCH], [1], [Define if sndtouch is available])
fi

# Output files
AC_CONFIG_FILES([
Makefile
src/Makefile
])

AC_OUTPUT
