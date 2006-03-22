noinst_LTLIBRARIES = libtkio.la
libtkio_la_SOURCES = transferKio.cpp
libtkio_la_LIBADD = ../../core/libcore.la

## ---- KIO PLUGIN ----
## Create plugin library
kde_module_LTLIBRARIES = libkget_kiofactory.la

libkget_kiofactory_la_SOURCES = transferKioFactory.cpp
libkget_kiofactory_la_LIBADD = $(LIB_QTCORE) $(LIB_QTGUI) $(LIB_QT3SUPPORT) $(LIB_KIO) $(LIB_KDEUI) $(LIB_KDECORE) libtkio.la ../../conf/libconf.la
libkget_kiofactory_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)

## Install plugin definition
kde_services_DATA = kget_kiofactory.desktop
## --------------------


INCLUDES = $(all_includes) -I$(srcdir)/../..

METASOURCES = AUTO
