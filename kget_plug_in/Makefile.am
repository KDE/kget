INCLUDES = $(all_includes)

kde_module_LTLIBRARIES = khtml_kget.la

khtml_kget_la_METASOURCES = AUTO
khtml_kget_la_SOURCES = kget_plug_in.cpp kget_linkview.cpp links.cpp
khtml_kget_la_LIBADD =  $(LIB_KHTML)
khtml_kget_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module

KDE_ICON = AUTO

part_DATA = kget_plug_in.rc kget_plug_in.desktop
partdir = $(kde_datadir)/khtml/kpartplugins
