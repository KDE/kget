####### kdevelop will overwrite this part!!! (begin)##########


EXTRA_DIST = added.wav finished.wav finishedall.wav started.wav 

install-data-local:
	$(mkinstalldirs) $(kde_datadir)/kget/sounds/
	$(INSTALL_DATA) $(srcdir)/added.wav $(kde_datadir)/kget/sounds/added.waw
	$(mkinstalldirs) $(kde_datadir)/kget/sounds/
	$(INSTALL_DATA) $(srcdir)/finished.wav $(kde_datadir)/kget/sounds/finished.wav
	$(mkinstalldirs) $(kde_datadir)/kget/sounds/
	$(INSTALL_DATA) $(srcdir)/finishedall.wav $(kde_datadir)/kget/sounds/finishedall.wav
	$(mkinstalldirs) $(kde_datadir)/kget/sounds/
	$(INSTALL_DATA) $(srcdir)/started.wav $(kde_datadir)/kget/sounds/started.wav

uninstall-local:
	-rm -f $(kde_datadir)/kget/sounds/added.waw
	-rm -f $(kde_datadir)/kget/sounds/finished.wav
	-rm -f $(kde_datadir)/kget/sounds/finishedall.wav
	-rm -f $(kde_datadir)/kget/sounds/started.wav

####### kdevelop will overwrite this part!!! (end)############

data_DATA =	added.wav started.wav finished.wav finishedall.wav

noinst_HEADERS = $(data_DATA)

datadir = $(kde_datadir)/kget/sounds
