SUBDIRS = icons pics

noinst_LTLIBRARIES = libui.la

libui_la_SOURCES = sidebar.cpp viewscontainer.cpp mainview.cpp droptarget.cpp tray.cpp splash.cpp\
                   transferdetailsfrm.ui transferdetails.cpp

libui_la_LIBADD = $(LIB_KUTILS)

INCLUDES = -I$(srcdir)/.. $(all_includes)

METASOURCES = AUTO
