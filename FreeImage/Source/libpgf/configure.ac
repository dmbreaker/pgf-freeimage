AC_INIT(src/PGFimage.cpp)

dnl The library interface version
dnl see documentation for versioning in the libtool manual
dnl NOTE: these should only be updated before public releases

dnl The current version of the interface. If interfaces has been 
dnl added removed changed bump this
INTERFACE_CURRENT=1

dnl If the source has been changed bump this
INTERFACE_REVISION=0

dnl If any interfaces has been added since last release, bump this
dnl If any interfaces has been removed, set this to 0
dnl The age is always less than CURRENT
INTERFACE_AGE=0

INTERFACE_VERSION=$INTERFACE_CURRENT:$INTERFACE_REVISION:$INTERFACE_AGE

AC_SUBST(INTERFACE_VERSION)

PACKAGE_RELEASE=1

AC_SUBST(PACKAGE_RELEASE)

AM_INIT_AUTOMAKE(libpgf,6.09.44)
AC_PROG_LIBTOOL
AM_CONFIG_HEADER( config.h )
AC_PROG_CXX

AC_CHECK_PROG(DOTFOUND,dot, "YES", "NO")

AC_OUTPUT(Makefile
  src/Makefile
  include/Makefile
  doc/Makefile
  doc/Doxyfile
  libpgf.spec
  libpgf.pc
)