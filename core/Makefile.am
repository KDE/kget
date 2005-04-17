SUBDIRS = plugin .

noinst_LTLIBRARIES = libcore.la 

libcore_la_SOURCES = model.cpp scheduler.cpp \
                     job.cpp jobqueue.cpp  transfer.cpp \
                     transferaction.cpp transferhandler.cpp \
                     transfergroup.cpp transfergrouphandler.cpp

libcore_la_LIBADD = plugin/libplugin.la

INCLUDES = $(all_includes) -I$(srcdir)/..

METASOURCES = AUTO
