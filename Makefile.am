SUBDIRS = conf views panels . pics icons sounds kget_plug_in
INCLUDES = $(all_includes) -Iconf

bin_PROGRAMS = kget

kget_LDFLAGS = $(all_libraries) $(KDE_RPATH) -Lconf -Lviews -Lpanels
kget_LDADD   = $(LIB_KFILE) $(LIB_KDEUI) $(LIB_KDECORE) $(LIBSOCKET) -lkmdi -lconf -lviews -lpanels
kget_SOURCES = transferlist.cpp transfer.cpp slave.cpp kfileio.cpp \
               scheduler.cpp connection.cpp safedelete.cpp \
               viewinterface.cpp docking.cpp splash.cpp \
               browserbar.cpp kmainwidget.cpp kmainwidget_actions.cpp \
               main.cpp kget_iface.skel

noinst_HEADERS = transferlist.h transfer.h slave.h kfileio.h \
                 scheduler.h connection.h safedelete.h  \
                 viewinterface.h viewinterface_p.h kmainwidget.h \
                 kmainwidget_actions.h docking.h \
                 splash.h globals.h

kget_METASOURCES = AUTO

xdg_apps_DATA = kget.desktop

mimedir = $(kde_mimedir)/application
mime_DATA = x-kgetlist.desktop

EXTRA_DIST = kget.desktop $(mime_DATA)

stuffdir = $(kde_datadir)/kget
stuff_DATA = kgetui.rc eventsrc

KDE_ICON = AUTO

messages: rc.cpp
	$(EXTRACTRC) *.ui > rc.cpp
	$(EXTRACTRC) *.rc */*.rc >> rc.cpp
	$(XGETTEXT) *.cpp */*.cpp *.h -o $(podir)/kget.pot
