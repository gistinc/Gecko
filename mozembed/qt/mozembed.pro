TEMPLATE = lib

HEADERS = QMozEmbedExport.h \
        QMozView.h \
	QMozApp.h \
	../common/embed.h \
	../common/EmbeddingSetup.h \
	../common/WebBrowserChrome.h \
	../common/ConsoleListener.h \
	../common/ContentListener.h \
	../common/DOMEventListener.h

SOURCES = QMozView.cpp \
	QMozApp.cpp \
	../common/embed.cpp \
	../common/EmbeddingSetup.cpp \
	../common/WebBrowserChrome.cpp \
	../common/ConsoleListener.cpp \
	../common/ContentListener.cpp \
	../common/DOMEventListener.cpp

TARGET = qmozembed

CONFIG += qt static

unix:QMAKE_CXXFLAGS += -fshort-wchar -fno-rtti

DEFINES += XPCOM_GLUE=1
win32:DEFINES += XP_WIN=1
CONFIG(dll):DEFINES += BUILD_QMOZEMBED

INCLUDEPATH = $(GRE_HOME)/../include/xulapp \
	$(GRE_HOME)/../include/nspr \
	$(GRE_HOME)/../include/xpcom \
	$(GRE_HOME)/../include/string \
	$(GRE_HOME)/../include/profdirserviceprovider \
	$(GRE_HOME)/../include/embed_base \
	$(GRE_HOME)/../include/windowwatcher \
	$(GRE_HOME)/../include/webbrwsr \
	$(GRE_HOME)/../include/shistory \
	$(GRE_HOME)/../include/uriloader \
	$(GRE_HOME)/../include/dom \
	$(GRE_HOME)/../include/necko \
	$(GRE_HOME)/../include/widget \
	$(GRE_HOME)/../include/docshell \
	$(GRE_HOME)/../include/profile \
	$(GRE_HOME)/../include \
	$(GRE_HOME)/../include/gfx \
	$(GRE_HOME)/../include/pref \
	../common

LIBS += -L$(GRE_HOME)/../lib

win32:LIBS += -ladvapi32

LIBS += -lxpcomglue 
