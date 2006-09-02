add_definitions(-DQT3_SUPPORT)

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

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} khtml_kget)

target_link_libraries(khtml_kget ${KDE4_KDECORE_LIBS} ${KDE4_KPARTS_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KHTML_LIBS})

install(TARGETS khtml_kget DESTINATION ${LIB_INSTALL_DIR}/kde4)


########### install files ###############

install(FILES kget_plug_in.rc DESTINATION ${DATA_INSTALL_DIR}/khtml/kpartplugins)
