lib_LTLIBRARIES = libkget_plug_in.la
INCLUDES = $(all_includes)
LDFLAGS =  
libkget_plug_in_la_METASOURCES = AUTO
LDADD =  $(LIB_QT) $(LIB_KDECORE) $(LIB_KDEUI) $(LIB_KPARTS) $(LIB_KHTML)
libkget_plug_in_la_SOURCES = kget_plug_in.cpp 
libkget_plug_in_la_LIBADD =  $(LIB_QT) $(LIB_KDECORE) $(LIB_KDEUI) $(LIB_KPARTS) $(LIB_KHTML)

libkget_plug_in_la_LDFLAGS = $(all_libraries) -avoid-version -module -no-undefined



EXTRA_DIST = kget_plug_in.cpp kget_plug_in.h kget_plug_in.rc  hi22-app-kget_plug_in.png 

install-data-local:
	$(mkinstalldirs)  $(kde_datadir)/khtml/kpartplugins/
	$(INSTALL_DATA) $(srcdir)/kget_plug_in.rc  $(kde_datadir)/khtml/kpartplugins/kget_plug_in.rc
	$(mkinstalldirs) $(kde_icondir)/hicolor/22x22/actions/
	$(INSTALL_DATA) $(srcdir)/hi22-app-kget_plug_in.png $(kde_icondir)/hicolor/22x22/actions/kget_plug_in.png


uninstall-local:
	-rm -f  $(kde_datadir)/khtml/kpartplugins/kget_plug_in.rc
	-rm -f $(kde_icondir)/locolor/16x16/actions/kget_plug_in.png
	-rm -f $(kde_icondir)/hicolor/16x16/actions/kget_plug_in.png

messages:
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
		 $(XGETTEXT) $$LIST -o ../po/kget_plug_in.pot; \
	fi

