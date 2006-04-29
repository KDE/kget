kde4_header()

install_files(${SERVICETYPES_INSTALL_DIR} FILES kget_plugin.desktop)

kde4_footer()

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
