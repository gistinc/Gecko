dnl ***** BEGIN LICENSE BLOCK *****
dnl  Version: MPL 1.1/GPL 2.0/LGPL 2.1
dnl
dnl   The contents of this file are subject to the Mozilla Public License Version
dnl   1.1 (the "License"); you may not use this file except in compliance with
dnl   the License. You may obtain a copy of the License at
dnl   http://www.mozilla.org/MPL/
dnl
dnl   Software distributed under the License is distributed on an "AS IS" basis,
dnl   WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
dnl   for the specific language governing rights and limitations under the
dnl   License.
dnl
dnl   The Original Code is mozilla.org code.
dnl
dnl   The Initial Developer of the Original Code is
dnl   Pioneer Research Center USA, Inc.
dnl   Portions created by the Initial Developer are Copyright (C) 2008
dnl   the Initial Developer. All Rights Reserved.
dnl
dnl   Contributor(s):
dnl      Andrey Petrov <andrey.petrov@gmail.com>
dnl
dnl   Alternatively, the contents of this file may be used under the terms of
dnl   either the GNU General Public License Version 2 or later (the "GPL"), or
dnl   the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
dnl   in which case the provisions of the GPL or the LGPL are applicable instead
dnl   of those above. If you wish to allow use of your version of this file only
dnl   under the terms of either the GPL or the LGPL, and not to allow others to
dnl   use your version of this file under the terms of the MPL, indicate your
dnl   decision by deleting the provisions above and replace them with the notice
dnl   and other provisions required by the GPL or the LGPL. If you do not delete
dnl   the provisions above, a recipient may use your version of this file under
dnl   the terms of any one of the MPL, the GPL or the LGPL.
dnl
dnl   ***** END LICENSE BLOCK *****

AC_INIT(configure.ac)
AM_CONFIG_HEADER(config.h)

if test -z "$VERSION"; then
VERSION=0.0.1-1
fi

AM_INIT_AUTOMAKE(libmozwebview, $VERSION)

AC_ISC_POSIX
AC_PROG_CC
AC_PROG_CXX
AM_PROG_CC_STDC
AC_PROG_INSTALL
AC_PROG_RANLIB
AC_CHECK_TOOL(AR, ar)
AC_HEADER_STDC
AM_PROG_LIBTOOL

PKG_CHECK_MODULES(GTK, [gtk+-2.0 >= 2.0.0], [], [AC_MSG_ERROR([gtk+-2.0 is required])])
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)

PKG_CHECK_MODULES(LIBXUL_EMBEDDING, [libxul-embedding], [], [AC_MSG_ERROR([libxul-embedding is required])])
AC_SUBST(LIBXUL_EMBEDDING_CFLAGS)
AC_SUBST(LIBXUL_EMBEDDING_LIBS)

dnl marshal generator
GLIB_GENMARSHAL="`$PKG_CONFIG --variable=glib_genmarshal glib-2.0`"
AC_SUBST(GLIB_GENMARSHAL)

LIBMOZWEBVIEW_REQUIRES="gtk+-2.0 libxul-embedding-unstable"
PACKAGE_CFLAGS="$MOZILLA_JS_CFLAGS $LIBXUL_EMBEDDING_CFLAGS $GTK_CFLAGS"

AC_SUBST(LIBMOZWEBVIEW_REQUIRES)
AC_SUBST(PACKAGE_CFLAGS)

AC_OUTPUT([
Makefile
libmozwebview.pc
])
