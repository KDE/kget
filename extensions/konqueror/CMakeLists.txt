project (kget_browser_integration)

set(kget_browser_integration_SRCS
   kget_plug_in.cpp
)

QT5_ADD_DBUS_INTERFACE(kget_browser_integration_SRCS ../../dbus/org.kde.kget.main.xml kget_interface )

add_library(kget_browser_integration MODULE ${kget_browser_integration_SRCS})

target_link_libraries(kget_browser_integration KF5::CoreAddons KF5::IconThemes KF5::Parts)

install(TARGETS kget_browser_integration DESTINATION ${KDE_INSTALL_PLUGINDIR})

########### install files ###############
install(FILES kget_plug_in.rc kget_plug_in.desktop DESTINATION ${KDE_INSTALL_DATADIR}/khtml/kpartplugins)
install(FILES kget_plug_in.rc kget_plug_in.desktop DESTINATION ${KDE_INSTALL_DATADIR}/dolphinpart/kpartplugins)
install(FILES kget_plug_in.rc kget_plug_in.desktop DESTINATION ${KDE_INSTALL_DATADIR}/kwebkitpart/kpartplugins)
