kde4_header()

install_files(${DATA_INSTALL_DIR}/kget/pics FILES
   kget_splash.png
   target.png
)

kde4_footer()

#original Makefile.am contents follow:

#pics_DATA = target.png kget_splash.png
#
#picsdir = $(kde_datadir)/kget/pics
#
#EXTRA_DIST = $(pics_DATA)
