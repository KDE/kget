kcoreaddons_add_plugin(krunner_kget)

set(krunner_kget_dbus_SRCS)
qt5_add_dbus_interface(krunner_kget_dbus_SRCS ../../dbus/org.kde.kget.main.xml kget_interface)

target_sources(krunner_kget PRIVATE
    ${krunner_kget_dbus_SRCS}
    kgetrunner.cpp
)

target_link_libraries(krunner_kget
    ${KDE4_PLASMA_LIBS}
)

install(TARGETS
    krunner_kget
    DESTINATION ${KDE_INSTALL_PLUGINDIR}
)
install(FILES
    plasma-runner-kget.desktop
    DESTINATION ${KDE_INSTALL_KSERVICES5DIR}
)
