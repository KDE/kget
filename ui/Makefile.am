SUBDIRS = icons pics

noinst_LTLIBRARIES = libui.la

libui_la_SOURCES = panelsmanager.cpp mainview.cpp splash.cpp

#droptarget.cpp groupspanel.cpp mainview.cpp panelsmanager.cpp \
#                splash.cpp testview.cpp tray.cpp traytransfer.cpp sidebar.cpp

libui_la_LIBADD = $(LIB_KUTILS)

INCLUDES = -I$(srcdir)/.. $(all_includes)

METASOURCES = AUTO
