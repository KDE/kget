kcoreaddons_add_plugin(plasma_engine_kget)

target_include_directories(plasma_engine_kget PRIVATE ../../)

set(kget_engine_dbus_SRCS)
qt5_add_dbus_interface(kget_engine_dbus_SRCS ../../dbus/org.kde.kget.main.xml kget_interface)

target_sources(plasma_engine_kget PRIVATE
    kgetengine.cpp
    ${kget_engine_dbus_SRCS}
)

target_link_libraries(plasma_engine_kget ${KDE4_KDECORE_LIBS} ${KDE4_PLASMA_LIBS})

install(TARGETS plasma_engine_kget DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES plasma-engine-kget.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
