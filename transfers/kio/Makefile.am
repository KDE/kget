## Create plugin library
#kde_module_LTLIBRARIES = libkget_kiotransfer.la
#
#libkget_kiotransfer_la_SOURCES = transferKio.cpp
#libkget_kiotransfer_la_LIBADD = $(top_builddir)/kget/core/libcore.la $(top_builddir)/kget/conf/libconf.la -lkdeui -lkdecore -lkio
#libkget_kiotransfer_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
#
## Install plugin definition
#kde_services_DATA = kget_kiotransfer.desktop

noinst_LTLIBRARIES = libtkio.la

libtkio_la_SOURCES = transferKio.cpp

INCLUDES = $(all_includes) -I$(srcdir)/../..

METASOURCES = AUTO
