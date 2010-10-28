project (kget_browser_integration)

set(kget_browser_integration_SRCS
   kget_plug_in.cpp
)

QT4_ADD_DBUS_INTERFACE(kget_browser_integration_SRCS ../../dbus/org.kde.kget.main.xml kget_interface )

kde4_add_plugin(kget_browser_integration ${kget_browser_integration_SRCS})

target_link_libraries(kget_browser_integration ${KDE4_KHTML_LIBS} ${KDE4_KIO_LIBRARY})

install(TARGETS kget_browser_integration DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############
install(FILES kget_plug_in.rc kget_plug_in.desktop DESTINATION ${DATA_INSTALL_DIR}/khtml/kpartplugins)
install(FILES kget_plug_in.rc kget_plug_in.desktop DESTINATION ${DATA_INSTALL_DIR}/dolphinpart/kpartplugins)
install(FILES kget_plug_in.rc kget_plug_in.desktop DESTINATION ${DATA_INSTALL_DIR}/kwebkitpart/kpartplugins)
