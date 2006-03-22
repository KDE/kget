if HAVE_LIBTORRENT
kde_module_LTLIBRARIES = libkget_bittorrentfactory.la

libkget_bittorrentfactory_la_SOURCES =  bttransfer.cpp http.cpp btthread.cpp bttransferhandler.cpp\
	bttransferfactory.cpp btdetailswidgetfrm.ui btdetailswidget.cpp
libkget_bittorrentfactory_la_LIBADD = $(LIB_QTCORE) $(LIB_QTGUI) $(LIB_QT3SUPPORT) $(LIB_KDEUI) $(LIB_KDECORE) $(LIB_KIO) $(TORRENT_LIBS) ../../core/libcore.la \
	../../conf/libconf.la
libkget_bittorrentfactory_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) \
	$(all_libraries)

noinst_HEADERS = bttransfer.h http.h btthread.h bttransferfactory.h bttransferhandler.h

INCLUDES = $(TORRENT_CFLAGS) $(all_includes) -I$(srcdir)/../.. 

kde_services_DATA = kget_bittorrentfactory.desktop

endif

METASOURCES = AUTO