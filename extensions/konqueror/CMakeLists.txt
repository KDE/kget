include(MacroOptionalDependPackage)

MACRO_OPTIONAL_DEPEND_PACKAGE(KdeWebKit "kdewebkit")
if(DEPEND_PACKAGE_KdeWebKit)
        macro_optional_find_package(KdeWebKit QUIET)
else(DEPEND_PACKAGE_KdeWebKit)
        set(KDEWEBKIT_FOUND FALSE)
endif(DEPEND_PACKAGE_KdeWebKit)

MACRO_OPTIONAL_DEPEND_PACKAGE(WebKitPart "webkitpart")
if(DEPEND_PACKAGE_WebKitPart)
        macro_optional_find_package(WebKitPart QUIET)
else(DEPEND_PACKAGE_WebKitPart)
        set(WEBKITPART_FOUND FALSE)
endif(DEPEND_PACKAGE_WebKitPart)

if( KDEWEBKIT_FOUND AND WEBKITPART_FOUND )
    add_definitions(-DHAVE_WEBKITKDE)
    include_directories(${KDEWEBKIT_INCLUDE_DIR})
    include_directories(${WEBKITPART_INCLUDE_DIR})
    FIND_PATH(WEBVIEW_PATH webkitkde/webview.h PATH ${KDEWEBKIT_INCLUDE_DIR})
    macro_bool_to_01(WEBVIEW_PATH HAVE_WEBVIEW )
    set(WEBKITKDE_FOUND true)
endif( KDEWEBKIT_FOUND AND WEBKITPART_FOUND )


macro_log_feature(WEBKITKDE_FOUND "WebKitKde" "Webkitkde libraries and header files" "" FALSE "" "Webkitkde is necessary to compile support for WebKitKde.")

configure_file(config-kgetplugins.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kgetplugins.h )


set(khtml_kget_SRCS
   kget_plug_in.cpp
   links.cpp
)

QT4_ADD_DBUS_INTERFACE(khtml_kget_SRCS ../../org.kde.kget.xml kget_interface )


kde4_add_plugin(khtml_kget ${khtml_kget_SRCS})

target_link_libraries(khtml_kget ${KDE4_KHTML_LIBS})

if( WEBKITKDE_FOUND)
    target_link_libraries(khtml_kget ${WEBKITPART_LIBRARIES} ${KDEWEBKIT_LIBRARIES} ${QT_QTWEBKIT_LIBRARY})
endif(WEBKITKDE_FOUND)

install(TARGETS khtml_kget DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install(FILES kget_plug_in.rc DESTINATION ${DATA_INSTALL_DIR}/khtml/kpartplugins)

if (WEBKITKDE_FOUND)
    install(FILES kget_plug_in.rc DESTINATION ${DATA_INSTALL_DIR}/webkitpart/kpartplugins)
endif (WEBKITKDE_FOUND)
