####### kdevelop will overwrite this part!!! (begin)##########
bin_PROGRAMS = kget

## INCLUDES were found outside kdevelop specific part

kget_SOURCES = getfilejob.cpp slaveevent.cpp slave.cpp transferlist.cpp transfer.cpp settings.cpp misc.cpp logwindow.cpp kmainwidget.cpp kfileio.cpp droptarget.cpp docking.cpp dlgSystem.cpp dlgPreferences.cpp dlgLimits.cpp dlgIndividual.cpp dlgDirectories.cpp dlgConnection.cpp dlgAutomation.cpp dlgAdvanced.cpp bwdatetime.cpp kget.cpp main.cpp 
kget_LDADD   = -lqt-mt $(LIB_KFILE) $(LIB_KDEUI) $(LIB_KDECORE) $(LIBSOCKET)

SUBDIRS = pics 

EXTRA_DIST = main.cpp kget.cpp kget.h kget.desktop lo32-app-kget.png lo16-app-kget.png bwdatetime.cpp bwdatetime.h common.h dlgAdvanced.cpp dlgAdvanced.h dlgAutomation.cpp dlgAutomation.h dlgConnection.cpp dlgConnection.h dlgDirectories.cpp dlgDirectories.h dlgIndividual.cpp dlgIndividual.h dlgLimits.cpp dlgLimits.h dlgPreferences.cpp dlgPreferences.h dlgSystem.cpp dlgSystem.h docking.cpp docking.h droptarget.cpp droptarget.h kfileio.cpp kfileio.h kmainwidget.cpp kmainwidget.h logwindow.cpp logwindow.h misc.cpp misc.h settings.cpp settings.h transfer.cpp transfer.h transferlist.cpp transferlist.h main.h version.h slave.cpp slave.h slaveevent.cpp slaveevent.h http_defaults.h kgetui.rc getfilejob.cpp getfilejob.h 

install-data-local:
	$(mkinstalldirs) $(kde_appsdir)/Applications/
	$(INSTALL_DATA) $(srcdir)/kget.desktop $(kde_appsdir)/Applications/kget.desktop
	$(mkinstalldirs) $(kde_icondir)/locolor/32x32/apps/
	$(INSTALL_DATA) $(srcdir)/lo32-app-kget.png $(kde_icondir)/locolor/32x32/apps/kget.png
	$(mkinstalldirs) $(kde_icondir)/locolor/16x16/apps/
	$(INSTALL_DATA) $(srcdir)/lo16-app-kget.png $(kde_icondir)/locolor/16x16/apps/kget.png

uninstall-local:
	-rm -f $(kde_appsdir)/Applications/kget.desktop
	-rm -f $(kde_icondir)/locolor/32x32/apps/kget.png
	-rm -f $(kde_icondir)/locolor/16x16/apps/kget.png

####### kdevelop will overwrite this part!!! (end)############
# this 10 paths are KDE specific. Use them:
# kde_htmldir       Where your docs should go to. (contains lang subdirs)
# kde_appsdir       Where your application file (.kdelnk) should go to. 
# kde_icondir       Where your icon should go to.
# kde_minidir       Where your mini icon should go to.
# kde_datadir       Where you install application data. (Use a subdir)
# kde_locale        Where translation files should go to.(contains lang subdirs)
# kde_cgidir        Where cgi-bin executables should go to.
# kde_confdir       Where config files should go to.
# kde_mimedir       Where mimetypes should go to.
# kde_toolbardir    Where general toolbar icons should go to.
# kde_wallpaperdir  Where general wallpapers should go to.

# set the include path for X, qt and KDE
INCLUDES= $(all_includes)

METASOURCES = AUTO

# the library search path. 
kget_LDFLAGS = $(all_libraries) $(KDE_RPATH)

# Uncomment the following two lines if you add a ui.rc file for your application to make use of
# KDE
rcdir = $(kde_datadir)/kget
rc_DATA = kgetui.rc

#WARNING: if you use a ui.rc file above, use:

# messages: rc.cpp

# instead of 

# messages:

messages: 
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
	 $(XGETTEXT) $$LIST -o $(podir)/kget.pot; \
	fi

