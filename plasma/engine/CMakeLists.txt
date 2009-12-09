include_directories( ../../)

set(kget_engine_SRCS
    kgetengine.cpp
)

qt4_add_dbus_interface(kget_engine_SRCS ../../../kget/dbus/org.kde.kget.main.xml kget_interface)

kde4_add_plugin(plasma_engine_kget ${kget_engine_SRCS})
target_link_libraries(plasma_engine_kget ${KDE4_KDECORE_LIBS} ${KDE4_PLASMA_LIBS})

install(TARGETS plasma_engine_kget DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-engine-kget.desktop DESTINATION ${SERVICES_INSTALL_DIR})
