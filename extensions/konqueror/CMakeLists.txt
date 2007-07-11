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

QT4_ADD_DBUS_INTERFACE(khtml_kget_SRCS ${CMAKE_SOURCE_DIR}/kget/org.kde.kget.xml kget_interface )


kde4_add_plugin(khtml_kget ${khtml_kget_SRCS})

target_link_libraries(khtml_kget ${KDE4_KHTML_LIBS})

install(TARGETS khtml_kget DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install(FILES kget_plug_in.rc DESTINATION ${DATA_INSTALL_DIR}/khtml/kpartplugins)
