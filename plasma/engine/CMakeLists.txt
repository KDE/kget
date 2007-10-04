include_directories(${PLASMA_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/kget)

set(kget_engine_SRCS
    kgetengine.cpp
)

qt4_add_dbus_interface(kget_engine_SRCS ../../../kget/org.kde.kget.xml kget_interface)

kde4_add_plugin(plasma_engine_kget ${kget_engine_SRCS})
target_link_libraries(plasma_engine_kget ${KDE4_KDECORE_LIBS} ${PLASMA_LIBS})

install(TARGETS plasma_engine_kget DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-engine-kget.desktop DESTINATION ${SERVICES_INSTALL_DIR})
