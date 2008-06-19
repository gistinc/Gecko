CC=gcc
CXX=g++

MOZ_CFLAGS=-DMOZ_DLL_SUFFIX="\".so\"" `pkg-config --cflags libxul libxul-embedding-unstable mozilla-js`
MOZ_LIBS=`pkg-config --libs libxul libxul-embedding-unstable mozilla-js`


SRCS = \
	test.cpp \
	moz-web-view.cpp \
	../common/embed.cpp \
	../common/WebBrowserChrome.cpp \
	../common/EmbeddingSetup.cpp \
	../common/ContentListener.cpp \
	nsGlueLinkingDlopen.cpp \
	nsXPCOMGlue.cpp

HEADERS = \
	../common/embed.h \
	nsGlueLinking.h \
	nsXPCOMGlue.h

CFLAGS=-fno-rtti -I../common `pkg-config --cflags gtk+-2.0` $(MOZ_CFLAGS)
LDFLAGS=`pkg-config --libs gtk+-2.0` $(MOZ_LIBS)

test: $(SRCS) $(HEADERS)
	$(CXX) -g $(CFLAGS) $(SRCS) $(LDFLAGS) -o test

all: test
clean:
	rm test