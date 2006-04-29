kde4_header()

add_subdirectory(plugin)

kde4_footer()

#original Makefile.am contents follow:

#SUBDIRS = plugin .
#
#noinst_LTLIBRARIES = libcore.la 
#
#libcore_la_SOURCES = model.cpp scheduler.cpp \
#                     job.cpp jobqueue.cpp  transfer.cpp \
#                     transferhandler.cpp \
#                     transfergroup.cpp transfergrouphandler.cpp
#
#libcore_la_LIBADD = plugin/libplugin.la
#
#INCLUDES = $(all_includes) -I$(qt_includes)/QtXml -I$(srcdir)/.. 
#
#METASOURCES = AUTO
