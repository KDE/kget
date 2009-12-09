include(MacroOptionalDependPackage)

MACRO_OPTIONAL_DEPEND_PACKAGE(WebKitPart "webkitpart")
if(DEPEND_PACKAGE_WebKitPart)
        macro_optional_find_package(WebKitPart QUIET)
else(DEPEND_PACKAGE_WebKitPart)
        set(WEBKITPART_FOUND FALSE)
endif(DEPEND_PACKAGE_WebKitPart)

if(WEBKITPART_FOUND)
    add_definitions(-DHAVE_WEBKITPART)
    include_directories(${WEBKITPART_INCLUDE_DIR})
    set(WEBKITPART_FOUND true)
endif(WEBKITPART_FOUND)


macro_log_feature(WEBKITPART_FOUND "WebKitPart" "WebKitPart libraries and header files" "https://svn.kde.org/home/kde/trunk/playground/libs/webkitkde" FALSE "" "WebKitPart is necessary to compile KGet support for WebKitPart.")

configure_file(config-kgetplugins.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kgetplugins.h )


set(khtml_kget_SRCS
   kget_plug_in.cpp
   links.cpp
)

QT4_ADD_DBUS_INTERFACE(khtml_kget_SRCS ../../dbus/org.kde.kget.main.xml kget_interface )


kde4_add_plugin(khtml_kget ${khtml_kget_SRCS})

target_link_libraries(khtml_kget ${KDE4_KHTML_LIBS})

if( WEBKITPART_FOUND)
    target_link_libraries(khtml_kget ${WEBKITPART_LIBRARIES} ${KDE4_KDEUI_LIBRARY} ${QT_QTWEBKIT_LIBRARY})
endif(WEBKITPART_FOUND)

install(TARGETS khtml_kget DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install(FILES kget_plug_in.rc DESTINATION ${DATA_INSTALL_DIR}/khtml/kpartplugins)

if (WEBKITPART_FOUND)
    install(FILES kget_plug_in.rc DESTINATION ${DATA_INSTALL_DIR}/webkitpart/kpartplugins)
endif (WEBKITPART_FOUND)
