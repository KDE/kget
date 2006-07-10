install(FILES kget_plugin.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})

#original Makefile.am contents follow:

#INCLUDES = -I$(srcdir)/.. -I$(srcdir)/../.. $(all_includes)
#
#noinst_LTLIBRARIES = libplugin.la
#libplugin_la_SOURCES = plugin.cpp transferfactory.cpp
#
#noinst_HEADERS = plugin.h transferfactory.h
#
#kde_servicetypes_DATA = kget_plugin.desktop
#
#METASOURCES = AUTO
