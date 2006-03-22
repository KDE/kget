SUBDIRS = conf core transfers ui sounds . plugins
INCLUDES = $(all_includes) -Iconf

bin_PROGRAMS = kget

kget_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kget_LDADD   = $(LIB_QTCORE) $(LIB_QTGUI) $(LIB_KIO) $(LIB_KDEUI) $(LIB_KDECORE) $(LIB_KUTILS) conf/libconf.la core/libcore.la ui/libui.la transfers/kio/libtkio.la
kget_SOURCES = dcopiface.skel kget.cpp main.cpp

noinst_HEADERS = kget.h

kget_METASOURCES = AUTO

xdg_apps_DATA = kget.desktop

mimedir = $(kde_mimedir)/application
mime_DATA = x-kgetlist.desktop

stuffdir = $(kde_datadir)/kget
stuff_DATA = kgetui.rc

servicemenudir = $(kde_datadir)/konqueror/servicemenus
servicemenu_DATA = kget_download.desktop

EXTRA_DIST = kget.desktop $(mime_DATA)

KDE_ICON = AUTO

messages: rc.cpp
	$(EXTRACTRC) *.ui > rc.cpp
	$(EXTRACTRC) *.rc */*.rc >> rc.cpp
	$(XGETTEXT) *.cpp */*.cpp *.h -o $(podir)/kget.pot
