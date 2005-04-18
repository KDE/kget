if HAVE_LIBTORRENT
kde_module_LTLIBRARIES = libkgetbittorrent.la

libkgetbittorrent_la_SOURCES =  bttransfer.cpp http.cpp btthread.cpp \
	bttransferfactory.cpp
libkgetbittorrent_la_LIBADD = $(LIB_KIO) $(TORRENT_LIBS) ../../core/libcore.la \
	../../conf/libconf.la
libkgetbittorrent_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) \
	$(all_libraries)

noinst_HEADERS = bttransfer.h http.h btthread.h bttransferfactory.h

INCLUDES = $(TORRENT_CFLAGS) $(all_includes) -I$(srcdir)/../.. 

kde_services_DATA = kget_bittorrentfactory.desktop

#METASOURCES = AUTO
endif