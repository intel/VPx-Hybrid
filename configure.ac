# Version notes:

m4_define([mediasdk_major_version], [1])
m4_define([mediasdk_minor_version], [12])
m4_define([mediasdk_micro_version], [2])

m4_define([mediasdk_version],
          [mediasdk_major_version.mediasdk_minor_version.mediasdk_micro_version])

# Libraries versions are set in the following way:
#  mediasdk_major_version = 1 (Media SDK API major version number)
#  mediasdk_minor_version = <number=8,9,10,...> (Media SDK API minor version number)
#  mediasdk_micro_version = <release number>
m4_define([mediasdk_lt_cur],
          [mediasdk_major_version])
m4_define([mediasdk_lt_rev],
          [mediasdk_minor_version])
m4_define([mediasdk_lt_age],
          [mediasdk_micro_version])

AC_PREREQ(2.63)
AC_INIT([mediasdk], [mediasdk_version], [dmitry.v.rogozhkin@intel.com], mediasdk)
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([Makefile.am])

AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE([dist-bzip2])
AM_CONFIG_HEADER([config.h])

#########################################
# Options for the ./configure we support.
#########################################
AC_ARG_ENABLE(private_build,
  [AC_HELP_STRING([--enable-private-build],
                  [turn on building of private targets from sources @<:@default=no@:>@])],
  [], [enable_private_build="no"])

AC_ARG_ENABLE(debug,
  [AC_HELP_STRING([--enable-debug],
                  [turn on debugging])],
  [], [debug="no"])

#########################################

AC_DISABLE_STATIC
AC_PROG_LIBTOOL
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL

AC_LIBTOOL_DLOPEN

AC_HEADER_STDC
AC_SYS_LARGEFILE

AC_TYPE_UINT32_T

###################################
# Passing user-flags to Makefile.am
###################################
ENABLE_PRIVATE_BUILD="no"

if test "$enable_private_build" = "yes"; then
  ENABLE_PRIVATE_BUILD="yes"
fi
AM_CONDITIONAL(ENABLE_PRIVATE_BUILD, test "$ENABLE_PRIVATE_BUILD" = "yes")

DEBUG="no"
if test "$debug" = "yes"; then
  DEBUG="yes"
fi
AM_CONDITIONAL(DEBUG, test "$DEBUG" = "yes")

###################################

MEDIASDK_MAJOR_VERSION=mediasdk_major_version
MEDIASDK_MINOR_VERSION=mediasdk_minor_version
MEDIASDK_MICRO_VERSION=mediasdk_micro_version

MEDIASDK_LT_CUR=mediasdk_lt_cur
MEDIASDK_LT_REV=mediasdk_lt_rev
MEDIASDK_LT_AGE=mediasdk_lt_age

MEDIASDK_VERSION=$MEDIASDK_MAJOR_VERSION.$MEDIASDK_MINOR_VERSION.$MEDIASDK_MICRO_VERSION
MEDIASDK_LT_VERSION="$MEDIASDK_LT_CUR:$MEDIASDK_LT_REV:$MEDIASDK_LT_AGE"
MEDIASDK_LT_LDFLAGS="-version-number $MEDIASDK_LT_VERSION"

AC_SUBST([MEDIASDK_VERSION], [${MEDIASDK_VERSION}])
AC_SUBST([MEDIASDK_LT_VERSION], ["${MEDIASDK_LT_VERSION}"])
AC_SUBST([MEDIASDK_LT_LDFLAGS], ["${MEDIASDK_LT_LDFLAGS}"])

PKG_CHECK_MODULES([LIBVA], [libva >= 0.34])
#PKG_CHECK_MODULES([DRM], [libdrm])

MEDIASDK_MAJOR_VERSION=mediasdk_major_version
MEDIASDK_MINOR_VERSION=mediasdk_minor_version
MEDIASDK_MICRO_VERSION=mediasdk_micro_version

MEDIASDK_LT_CUR=mediasdk_lt_cur
MEDIASDK_LT_REV=mediasdk_lt_rev
MEDIASDK_LT_AGE=mediasdk_lt_age

MEDIASDK_VERSION=$MEDIASDK_MAJOR_VERSION.$MEDIASDK_MINOR_VERSION.$MEDIASDK_MICRO_VERSION
MEDIASDK_LT_VERSION="$MEDIASDK_LT_CUR:$MEDIASDK_LT_REV:$MEDIASDK_LT_AGE"
MEDIASDK_LT_LDFLAGS="-version-number $MEDIASDK_LT_VERSION"

AC_SUBST([MEDIASDK_VERSION], [${MEDIASDK_VERSION}])
AC_SUBST([MEDIASDK_LT_VERSION], ["${MEDIASDK_LT_VERSION}"])
AC_SUBST([MEDIASDK_LT_LDFLAGS], ["${MEDIASDK_LT_LDFLAGS}"])

BUILD_32="no"
BUILD_64="no"

case "$target_os" in
  linux*)
    case "$target_cpu" in
      i[[3456]]86)
        BUILD_32="yes"
        AC_SUBST([MFX_ARCH_CFLAGS], ['-DLINUX32'])
        ;;
      x86_64)
        BUILD_64="yes"
        AC_SUBST([MFX_ARCH_CFLAGS], ['-DLINUX32 -DLINUX64'])
        ;;
    esac
    ;;
esac

AM_CONDITIONAL(BUILD_32, test "$BUILD_32" = "yes")
AM_CONDITIONAL(BUILD_64, test "$BUILD_64" = "yes")

AC_SUBST([MEDIASDK_ROOT], ['${top_srcdir}'])

AC_SUBST(MFX_PRODUCT_VERSION, $MEDIASDK_VERSION)
AC_SUBST(MFX_FILE_VERSION, `echo $MFX_PRODUCT_VERSION | cut -f 1 -d.``date +.%-y.%-m.%-d`)

NO_WARNINGS="-Wno-unknown-pragmas -Wno-unused"

MFX_CFLAGS=
if test "$DEBUG" = "yes"; then
  AC_SUBST([MFX_CFLAGS], ["-D_DEBUG -Wall ${NO_WARNINGS}"])
else
  AC_SUBST([MFX_CFLAGS], ["-DNDEBUG -D_FORTIFY_SOURCE=2 -fstack-protector -msse4.2 -Wall ${NO_WARNINGS}"])
fi

AC_SUBST([MFX_CFLAGS], ["${MFX_ARCH_CFLAGS} ${MFX_CFLAGS}"])
AC_SUBST([MFX_CFLAGS], ["-DMFX_FILE_VERSION=\"${MFX_FILE_VERSION}\" -DMFX_PRODUCT_VERSION=\"${MFX_PRODUCT_VERSION}\" ${MFX_CFLAGS}"])

AC_SUBST([MFX_CFLAGS_INTERNAL], ['${MFX_CFLAGS}'])

MFX_LDFLAGS=

##########################################################
# Usage: MFX_CHECK_LDFLAGS([<flags>, <action on failure>])
# Notes:
#  - <flags> will be appended to the MFX_LDFLAGS variable
##########################################################
AC_DEFUN([MFX_CHECK_LDFLAGS], [
  saved_ldflags="$LDFLAGS"
  LDFLAGS="$1"
  AC_MSG_CHECKING(if linker supports $LDFLAGS)
  AC_LINK_IFELSE(
    [AC_LANG_PROGRAM(
      [[]],
      [[int main(){ return 0; }]])],
      [AC_MSG_RESULT(yes)
        AC_SUBST([MFX_LDFLAGS], ["$1 ${MFX_LDFLAGS}"])],
      [AC_MSG_RESULT(no)]
        $2)
  LDFLAGS="$saved_ldflags"
])
##########################################################

##############################
# Checking for the link flags:
# 1. mandatory flags - actually that's just to pay atention that somethings is not supported
# 2. flags which may be not supported
##############################
MFX_CHECK_LDFLAGS(
  [-Wl,--no-undefined,-z,relro,-z,now,-z,noexecstack -fstack-protector],
  [AC_MSG_FAILURE(build is not possible due to unsupported build flags)])
MFX_CHECK_LDFLAGS(
  [-Wl,--default-symver],
  [])
##############################

AC_SUBST([MFX_LDFLAGS], ["${MFX_LDFLAGS}"])

AC_CONFIG_FILES([
  Makefile
  _studio/Makefile
  _studio/ipp/Makefile
  _studio/mfx_lib/Makefile
  _studio/shared/Makefile
  _studio/shared/umc/Makefile
  _studio/shared/umc/core/Makefile
  _studio/shared/umc/io/Makefile
  _studio/vp8e/Makefile
  ])

AC_OUTPUT

# Print a small summary

echo
echo "mediasdk - $MFX_PRODUCT_VERSION"
echo
echo Media SDK file version ........... : $MFX_FILE_VERSION
echo Media SDK product version ........ : $MFX_PRODUCT_VERSION
echo Media SDK link flags ............. : $MFX_LDFLAGS
