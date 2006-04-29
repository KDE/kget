kde4_header()

install_files(${DATA_INSTALL_DIR}/kget FILES
   eventsrc
)
install_files(${SOUND_INSTALL_DIR} FILES
   KGet_Added.ogg
   KGet_Finished.ogg
   KGet_Finished_All.ogg
   KGet_Started.ogg
)

kde4_footer()

#original Makefile.am contents follow:

#events_DATA = eventsrc
#eventsdir = $(kde_datadir)/kget
#
#kde_sound_DATA = KGet_Added.ogg KGet_Finished.ogg KGet_Finished_All.ogg \
#                 KGet_Started.ogg
#
#EXTRA_DIST = $(kde_sound_DATA)
