SUBDIRS = . pics icons sounds kget_plug_in
INCLUDES = $(all_includes)


bin_PROGRAMS = kget

kget_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kget_LDADD   = $(LIB_KFILE) $(LIB_KDEUI) $(LIB_KDECORE) $(LIBSOCKET) -lkmdi
kget_SOURCES = slave.cpp transferlist.cpp \
               transfer.cpp settings.cpp logwindow.cpp \
               kmainwidget.cpp kfileio.cpp droptarget.cpp docking.cpp \
               dlgconnectionbase.ui dlgautomationbase.ui dlglimitsbase.ui \
               dlgadvancedbase.ui dlgdirectoriesbase.ui dlgsystembase.ui \
               dlgSystem.cpp dlgPreferences.cpp dlgLimits.cpp \
               dlgIndividual.cpp dlgDirectories.cpp dlgConnection.cpp \
               dlgAutomation.cpp dlgAdvanced.cpp \
               main.cpp dockindividual.cpp kget_iface.skel safedelete.cpp \
               scheduler.cpp viewinterface.cpp iconview.cpp

noinst_HEADERS = globals.h \
                 dlgAdvanced.h dlgAutomation.h dlgConnection.h \
                 dlgDirectories.h dlgIndividual.h dlgLimits.h \
                 dlgPreferences.h dlgSystem.h docking.h droptarget.h \
                 kfileio.h kmainwidget.h logwindow.h settings.h \
                 transfer.h transferlist.h slave.h \
                 dockindividual.h \
                 scheduler.h viewinterface.h iconview.h

kget_METASOURCES = AUTO

xdg_apps_DATA = kget.desktop

mimedir = $(kde_mimedir)/application
mime_DATA =x-kgetlist.desktop

EXTRA_DIST = kget.desktop $(mime_DATA)

rcdir = $(kde_datadir)/kget
rc_DATA = kgetui.rc

KDE_ICON = AUTO

messages: rc.cpp
	$(EXTRACTRC) *.ui > rc.cpp
	$(EXTRACTRC) *.rc */*.rc >> rc.cpp
	$(XGETTEXT) *.cpp */*.cpp *.h -o $(podir)/kget.pot
