AC_PREREQ([2.69])
AC_INIT([disgrace], [0.1.0], [miroslav.shaltev@shaltev.de])
AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_HEADERS([config.h])

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

# JACK
PKG_CHECK_MODULES([JACK], [jack >= 1.9.0])

# FLTK
PKG_CHECK_MODULES([FLTK], [fltk])

# libsndfile
PKG_CHECK_MODULES([SNDFILE], [sndfile])

# libsamplerate
PKG_CHECK_MODULES([SAMPLERATE], [samplerate])

# Optional: libsndtouch
AC_ARG_ENABLE([sndtouch],
    AS_HELP_STRING([--enable-sndtouch], [Enable time-stretch support]),
    [enable_sndtouch=yes],
    [enable_sndtouch=no]
)

if test "x$enable_sndtouch" = "xyes"; then
    PKG_CHECK_MODULES([SNDTOUCH], [sndtouch])
    AC_DEFINE([HAVE_SNDTOUCH], [1], [Define if sndtouch is available])
fi

# Output files
AC_CONFIG_FILES([
Makefile
src/Makefile
examples/Makefile
])

AC_OUTPUT
