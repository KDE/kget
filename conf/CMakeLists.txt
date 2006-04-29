kde4_header()

install_files(${KCFG_INSTALL_DIR} FILES kget.kcfg)

kde4_footer()

#original Makefile.am contents follow:

#noinst_LTLIBRARIES = libconf.la
#
#libconf_la_SOURCES = dlgappearance.ui dlgadvanced.ui dlgnetwork.ui \
#                     dlgdirectories.ui preferencesdialog.cpp \
#                     settings.kcfgc
#
#INCLUDES = -I$(srcdir)/.. -I$(srcdir)/../views $(all_includes)
#
#METASOURCES = AUTO
#
#kde_kcfg_DATA = kget.kcfg
