# ***** BEGIN LICENSE BLOCK *****
# Version: MPL 1.1/GPL 2.0/LGPL 2.1
#
#  The contents of this file are subject to the Mozilla Public License Version
#  1.1 (the "License"); you may not use this file except in compliance with
#  the License. You may obtain a copy of the License at
#  http://www.mozilla.org/MPL/
#
#  Software distributed under the License is distributed on an "AS IS" basis,
#  WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
#  for the specific language governing rights and limitations under the
#  License.
#
#  The Original Code is mozilla.org code.
#
#  The Initial Developer of the Original Code is
#  Pioneer Research Center USA, Inc.
#  Portions created by the Initial Developer are Copyright (C) 2008
#  the Initial Developer. All Rights Reserved.
#
#  Contributor(s):
#     Andrey Petrov <andrey.petrov@gmail.com>
#
#  Alternatively, the contents of this file may be used under the terms of
#  either the GNU General Public License Version 2 or later (the "GPL"), or
#  the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
#  in which case the provisions of the GPL or the LGPL are applicable instead
#  of those above. If you wish to allow use of your version of this file only
#  under the terms of either the GPL or the LGPL, and not to allow others to
#  use your version of this file under the terms of the MPL, indicate your
#  decision by deleting the provisions above and replace them with the notice
#  and other provisions required by the GPL or the LGPL. If you do not delete
#  the provisions above, a recipient may use your version of this file under
#  the terms of any one of the MPL, the GPL or the LGPL.
#
#  ***** END LICENSE BLOCK *****

INCLUDES = \
	-I../../common \
	-I../common \
	-I../include \
	@PACKAGE_CFLAGS@

lib_LTLIBRARIES = libmozwebview.la

libmozwebview_la_SOURCES = \
	../common/moz-web-view-marshal.c \
	moz-web-view.c \
	../../common/embed.cpp \
	../../common/WebBrowserChrome.cpp \
	../../common/EmbeddingSetup.cpp \
	../../common/ContentListener.cpp \
	../../common/DOMEventListener.cpp \
	../../common/ConsoleListener.cpp \
	../common/moz-web-view-common.cpp \
	$(NULL)

include_HEADERS = \
	$(top_srcdir)/../../common/embed.h \
	$(top_srcdir)/../include/moz-web-view.h
	$(NULL)

noinst_HEADERS = \
	$(srcdir)/../common/moz-web-view-marshal.h \
	$(NULL)

MOZ_CFLAGS = -DMOZ_DLL_SUFFIX="\".so\""

CFLAGS += $(MOZ_CFLAGS) $(GTK_CFLAGS) $(LIBXUL_EMBEDDING_CFLAGS) -fno-rtti
CPPFLAGS += $(CFLAGS)
LDFLAGS += $(GTK_LIBS) $(LIBXUL_EMBEDDING_LIBS)

pkgconfigdir = $(prefix)/lib/pkgconfig

pkgconfig_DATA = $(top_srcdir)/libmozwebview.pc

MARSHAL_FILE = ../common/moz-web-view-marshal
MARSHAL_PREFIX = g_cclosure_user_marshal

$(MARSHAL_FILE).h: $(MARSHAL_FILE).list
	@GLIB_GENMARSHAL@ --prefix=$(MARSHAL_PREFIX) $(srcdir)/$(MARSHAL_FILE).list --skip-source --header > $(MARSHAL_FILE).h

$(MARSHAL_FILE).c: $(MARSHAL_FILE).list $(MARSHAL_FILE).h
	@GLIB_GENMARSHAL@ --prefix=$(MARSHAL_PREFIX) $(srcdir)/$(MARSHAL_FILE).list --skip-source --body > $(MARSHAL_FILE).c
