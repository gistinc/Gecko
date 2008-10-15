# UNTESTED

CC=gcc
CXX=g++

MOZ_CFLAGS=-DMOZ_DLL_SUFFIX="\".so\"" `pkg-config --cflags libxul libxul-embedding-unstable mozilla-js`
MOZ_LIBS=`pkg-config --libs libxul libxul-embedding-unstable mozilla-js`

OBJECTS = \
	moz-web-view.o \
	../../common/embed.o \
	../../common/WebBrowserChrome.o \
	../../common/EmbeddingSetup.o \
	../../common/ContentListener.o \
	../common/moz-web-view-common.o \
	../common/moz-web-view-marshal.o \
	nsGlueLinkingDlopen.o \
	nsXPCOMGlue.o

CFLAGS=-fno-rtti -I../common `pkg-config --cflags gtk+-2.0` $(MOZ_CFLAGS)
LDFLAGS=`pkg-config --libs gtk+-2.0` $(MOZ_LIBS)

all: libmozwebviw.so
clean:
	rm *.o *.so test

# generate marshalers
$(OBJECTS):../common/moz-web-view-marshal.c ../common/moz-web-view-marshal.h

../common/moz-web-view-marshal.c:../common/moz-web-view-marshal.list
	glib-genmarshal --body > $< > $@

../common/moz-web-view-marshal.h:../common/moz-web-view-marshal.list
	glib-genmarshal --header $< > $@

%.o:%.c
	$(CC) -o $@ -c $< $(CFLAGS) 

%.o:%.cc
	$(CXX) -o $@ -c $< $(CFLAGS) 

libmozwebviw.so: $(OBJECTS)
	$(CXX) -o $@ --shared $^ $(LDFLAGS)
