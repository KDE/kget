include(MacroOptionalDependPackage)
macro_optional_depend_package(WebKitKde "webkitkde")
if (DEPEND_PACKAGE_WebKitKde)
    macro_optional_find_package(WebKitKde QUIET)
else (DEPEND_PACKAGE_WebKitKde)
    set(WEBKITKDE_FOUND FALSE)
endif (DEPEND_PACKAGE_WebKitKde)

if( WEBKITKDE_FOUND)
    add_definitions(-DHAVE_WEBKITKDE)
    include_directories(${WEBKITKDE_INCLUDE_DIR})
    FIND_PATH(WEBVIEW_PATH webkitkde/webview.h PATH ${WEBKITKDE_INCLUDE_DIR})
    macro_bool_to_01(WEBVIEW_PATH HAVE_WEBVIEW )
endif(WEBKITKDE_FOUND)

configure_file(config-kgetplugins.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kgetplugins.h )


set(khtml_kget_SRCS
   kget_plug_in.cpp
   links.cpp
)

QT4_ADD_DBUS_INTERFACE(khtml_kget_SRCS ../../org.kde.kget.xml kget_interface )


kde4_add_plugin(khtml_kget ${khtml_kget_SRCS})

target_link_libraries(khtml_kget ${KDE4_KHTML_LIBS})

if( WEBKITKDE_FOUND)
    target_link_libraries(khtml_kget ${WEBKITKDE_LIBRARIES} ${QT_QTWEBKIT_LIBRARY})
endif(WEBKITKDE_FOUND)

install(TARGETS khtml_kget DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install(FILES kget_plug_in.rc DESTINATION ${DATA_INSTALL_DIR}/khtml/kpartplugins)

if (WEBKITKDE_FOUND)
    install(FILES kget_plug_in.rc DESTINATION ${DATA_INSTALL_DIR}/webkitpart/kpartplugins)
endif (WEBKITKDE_FOUND)
