add_library(kget_browser_integration MODULE)

set(kget_browser_integration_dbus_SRCS)
qt5_add_dbus_interface(kget_browser_integration_dbus_SRCS ../../dbus/org.kde.kget.main.xml kget_interface )

target_sources(kget_browser_integration PRIVATE
   kget_plug_in.cpp
   ${kget_browser_integration_dbus_SRCS}
)

target_link_libraries(kget_browser_integration KF5::CoreAddons KF5::I18n KF5::IconThemes KF5::Parts)

install(TARGETS kget_browser_integration DESTINATION ${KDE_INSTALL_PLUGINDIR})

########### install files ###############
install(FILES kget_plug_in.rc kget_plug_in.desktop DESTINATION ${KDE_INSTALL_DATADIR}/khtml/kpartplugins)
install(FILES kget_plug_in.rc kget_plug_in.desktop DESTINATION ${KDE_INSTALL_DATADIR}/dolphinpart/kpartplugins)
install(FILES kget_plug_in.rc kget_plug_in.desktop DESTINATION ${KDE_INSTALL_DATADIR}/kwebkitpart/kpartplugins)
