MOZ_CFLAGS=-DMOZ_DLL_SUFFIX="\".so\"" `pkg-config --cflags libxul libxul-embedding-unstable mozilla-js`
MOZ_LIBS=`pkg-config --libs libxul libxul-embedding-unstable mozilla-js`

SRCS= \
	test.cpp \
	../common/embed.cpp \
	../common/WebBrowserChrome.cpp \
	nsGlueLinkingDlopen.cpp \
	nsXPCOMGlue.cpp

HEADERS = \
	../common/embed.h \
	nsGlueLinking.h \
	nsXPCOMGlue.h

test: $(SRCS) $(HEADERS) Makefile
	g++ $(SRCS) `pkg-config --cflags --libs gtk+-2.0` $(MOZ_CFLAGS) $(MOZ_LIBS) -o test
