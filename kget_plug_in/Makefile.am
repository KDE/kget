
lib_LTLIBRARIES = libkget_plug_in.la
INCLUDES = $(all_includes)

libkget_plug_in_la_METASOURCES = AUTO
LDADD =  $(LIB_KHTML)
libkget_plug_in_la_SOURCES = kget_plug_in.cpp 
libkget_plug_in_la_LIBADD =  $(LIB_KHTML)

libkget_plug_in_la_LDFLAGS = $(all_libraries) -avoid-version -module -no-undefined

KDE_ICON = kget_plugin

data_DATA = kget_plug_in.rc
datadir = $(kde_datadir)/khtml/kpartplugins

