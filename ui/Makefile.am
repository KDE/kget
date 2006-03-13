SUBDIRS = icons pics

noinst_LTLIBRARIES = libui.la

libui_la_SOURCES = viewscontainer.cpp mainview.cpp droptarget.cpp tray.cpp splash.cpp\
                   transferdetailsfrm.ui transferdetails.cpp

libui_la_LIBADD = $(LIB_KDEUI) $(LIB_KDECORE) $(LIB_KUTILS)
libui_la_LDFLAGS = $(all_libraries)

INCLUDES = -I$(srcdir)/.. $(all_includes)

METASOURCES = AUTO
