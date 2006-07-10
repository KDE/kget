include_directories(
   ${KDE4_INCLUDE_DIR}
   ${QT_INCLUDES}
)

########### next target ###############

set(khtml_kget_SRCS
   kget_linkview.cpp
   kget_plug_in.cpp
   links.cpp
)

kde4_automoc(${khtml_kget_SRCS})

kde4_add_plugin(khtml_kget ${khtml_kget_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} khtml_kget)

target_link_libraries(khtml_kget ${KDE4_KDECORE_LIBS} ${KDE4_KPARTS_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KHTML_LIBS} ${KDE4_DCOP_LIBRARY})

install(TARGETS khtml_kget DESTINATION ${LIB_INSTALL_DIR}/kde4)


########### install files ###############

install(FILES kget_plug_in.rc DESTINATION ${DATA_INSTALL_DIR}/khtml/kpartplugins)

kde4_install_icons(${ICON_INSTALL_DIR} hicolor)

#original Makefile.am contents follow:

#INCLUDES = $(all_includes)
#
#kde_module_LTLIBRARIES = khtml_kget.la
#
#khtml_kget_la_METASOURCES = AUTO
#khtml_kget_la_SOURCES = kget_plug_in.cpp kget_linkview.cpp links.cpp
#khtml_kget_la_LIBADD = $(LIB_QTCORE) $(LIB_QTGUI) $(LIB_QT3SUPPORT) $(LIB_KPARTS) $(LIB_KDEUI) $(LIB_KDECORE) $(LIB_KHTML) -lDCOP
#khtml_kget_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
#
#KDE_ICON = AUTO
#
#part_DATA = kget_plug_in.rc
#partdir = $(kde_datadir)/khtml/kpartplugins
