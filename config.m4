dnl $Id$
dnl config.m4 for extension fcgicli

PHP_ARG_ENABLE(fcgicli, whether to enable fcgicli support,
dnl Make sure that the comment is aligned:
[  --enable-fcgicli           Enable fcgicli support])

if test "$PHP_FCGICLI" != "no"; then
  PHP_REQUIRE_CXX()

  # --with-uv -> check with-path
  SEARCH_PATH="/usr/local /usr"
  SEARCH_FOR="/include/uv.h"
  if test -r $PHP_FCGICLI/$SEARCH_FOR; then # path given as parameter
    UV_DIR=$PHP_FCGICLI
  else # search default path list
    AC_MSG_CHECKING([for uv files in default path])
    for i in $SEARCH_PATH ; do
      if test -r $i/$SEARCH_FOR; then
        UV_DIR=$i
        AC_MSG_RESULT(found in $i)
      fi
    done
  fi

  if test -z "$UV_DIR"; then
    AC_MSG_RESULT([not found])
    AC_MSG_ERROR([Please reinstall the libuv distribution, "$PHP_FCGICLI"])
  fi

  # --with-uv -> add include path
  PHP_ADD_INCLUDE($UV_DIR/include)

  # --with-uv -> check for lib and symbol presence
  LIBNAME="uv"
  LIBSYMBOL="uv_version"

  PHP_SUBST(FCGICLI_SHARED_LIBADD)

  PHP_ADD_LIBRARY(stdc++, 1, FCGICLI_SHARED_LIBADD)
  PHP_ADD_LIBRARY(uv, 1, FCGICLI_SHARED_LIBADD)
  CFLAGS="-O3 -funroll-loops"
  CXXFLAGS="-pthread -std=c++14 -O3 -funroll-loops"

  PHP_NEW_EXTENSION(fcgicli, fcgicli.cc MultiRequest.cc fcgicli_api.cc fcgicli.c, $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
fi
