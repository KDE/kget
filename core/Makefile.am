SUBDIRS = plugin .

noinst_LTLIBRARIES = libcore.la 

libcore_la_SOURCES = connection.cpp group.cpp safedelete.cpp scheduler.cpp \
                     transfer.cpp transferlist.cpp viewinterface.cpp
libcore_la_LIBADD = plugin/libplugin.la

INCLUDES = $(all_includes) -I$(srcdir)/..

METASOURCES = AUTO
