
kde_module_LTLIBRARIES = libkget_multithreadedfactory.la

libkget_multithreadedfactory_la_SOURCES = transferMultiThreadedFactory.cpp transferMultiThreaded.cpp mthreaded.cpp thread.cpp mtdetailswidget.cpp mtdetailswidgetfrm.ui

libkget_multithreadedfactory_la_LIBADD = $(LIB_QTCORE) $(LIB_QTGUI) $(LIB_QT3SUPPORT) $(LIB_KDEUI) $(LIB_KDECORE) $(LIB_KIO) ../../conf/libconf.la ../../core/libcore.la

libkget_multithreadedfactory_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)

kde_services_DATA = kget_multithreadedfactory.desktop

INCLUDES = $(all_includes) -I$(qt_includes)/QtNetwork -I$(srcdir)/../..

METASOURCES = AUTO
