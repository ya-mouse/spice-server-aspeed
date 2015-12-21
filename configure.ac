AC_PREREQ([2.57])

# Follow the libtool manual for the so version:
# http://www.gnu.org/software/libtool/manual/html_node/Updating-version-info.html
# - If the library source code has changed at all since the last update,
#   then increment revision (‘c:r:a’ becomes ‘c:r+1:a’).
# - If any interfaces have been added, removed, or changed since the last update,
#   increment current, and set revision to 0.
# - If any interfaces have been added since the last public release,
#   then increment age.
# - If any interfaces have been removed or changed since the last public release,
#   then set age to 0.
#
AC_INIT(spice_server_aspeed, [m4_esyscmd(build-aux/git-version-gen .tarball-version)],
        [mouse@yandex.ru], spice_server)

AC_CONFIG_MACRO_DIR([m4])
AM_CONFIG_HEADER([config.h])
AC_CONFIG_AUX_DIR(.)

AM_INIT_AUTOMAKE([dist-bzip2 no-dist-gzip subdir-objects])
AM_MAINTAINER_MODE

AC_PROG_CC
AC_PROG_CC_C99
if test x"$ac_cv_prog_cc_c99" = xno; then
    AC_MSG_ERROR([C99 compiler is required.])
fi
AC_PROG_INSTALL
AC_CANONICAL_HOST
AC_PROG_LIBTOOL
AM_PROG_CC_C_O
AC_C_BIGENDIAN
PKG_PROG_PKG_CONFIG

AC_CHECK_HEADERS([sys/time.h])
AC_CHECK_HEADERS([execinfo.h])
AC_CHECK_HEADERS([linux/sockios.h])
AC_FUNC_ALLOCA

# For autoconf < 2.63
m4_ifndef([AS_VAR_APPEND],
          AC_DEFUN([AS_VAR_APPEND], $1=$$1$2))

# Check for the CPU we are using
case $host_cpu in
  x86_64)
        arch_warn=""
        ;;
  *)
        arch_warn="spice-server on non-x86_64 architectures hasn't been extensively tested"
esac

dnl =========================================================================
dnl Check deps

AC_SUBST(COMMON_CFLAGS)

PKG_CHECK_MODULES([SPICE_PROTOCOL], [spice-protocol])
PKG_CHECK_MODULES([SPICE_SERVER], [spice-server])
PKG_CHECK_MODULES([GLIB2], [glib-2.0])

AC_SUBST(WARN_CFLAGS)

dnl =========================================================================
dnl -fvisibility stuff

have_gcc4=no
AC_MSG_CHECKING(for -fvisibility)
AC_COMPILE_IFELSE([AC_LANG_SOURCE([
#if defined(__GNUC__) && (__GNUC__ >= 4)
#else
error Need GCC 4.0 for visibility
#endif
int main () { return 0; }
])], have_gcc4=yes)

if test "x$have_gcc4" = "xyes" && test ! "$os_win32" = "yes" ; then
   VISIBILITY_HIDDEN_CFLAGS="-fvisibility=hidden"
fi
AC_MSG_RESULT($have_gcc4)
AC_SUBST(VISIBILITY_HIDDEN_CFLAGS)

m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

AC_OUTPUT([
Makefile
])
