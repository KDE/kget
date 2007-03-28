include_directories(
   ${KDE4_INCLUDES}
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

target_link_libraries(khtml_kget ${KDE4_KPARTS_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KHTML_LIBS})

install(TARGETS khtml_kget DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install(FILES kget_plug_in.rc DESTINATION ${DATA_INSTALL_DIR}/khtml/kpartplugins)
