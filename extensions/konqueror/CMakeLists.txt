include(MacroOptionalDependPackage)

MACRO_OPTIONAL_DEPEND_PACKAGE(KWebKitPart "kwebkitpart")
if(DEPEND_PACKAGE_KWebKitPart)
        macro_optional_find_package(KWebKitPart QUIET)
else(DEPEND_PACKAGE_KWebKitPart)
        set(KWEBKITPART_FOUND FALSE)
endif(DEPEND_PACKAGE_KWebKitPart)

if(KWEBKITPART_FOUND)
    add_definitions(-DHAVE_KWEBKITPART)
    include_directories(${KWEBKITPART_INCLUDE_DIR})
    set(KWEBKITPART_FOUND true)
endif(KWEBKITPART_FOUND)


macro_log_feature(KWEBKITPART_FOUND "KWebKitPart" "KWebKitPart libraries and header files" "https://svn.kde.org/home/kde/trunk/extragear/base/kwebkitpart" FALSE "" "KWebKitPart is necessary to compile KGet support for KWebKitPart.")

set(kget_browser_integration_SRCS
   kget_plug_in.cpp
   links.cpp
)

QT4_ADD_DBUS_INTERFACE(kget_browser_integration_SRCS ../../dbus/org.kde.kget.main.xml kget_interface )


kde4_add_plugin(kget_browser_integration ${kget_browser_integration_SRCS})

target_link_libraries(kget_browser_integration ${KDE4_KHTML_LIBS} ${KDE4_KIO_LIBRARY})

if(KWEBKITPART_FOUND)
    target_link_libraries(kget_browser_integration ${KWEBKITPART_LIBRARIES} ${KDE4_KDEUI_LIBRARY} ${QT_QTWEBKIT_LIBRARY})
endif(KWEBKITPART_FOUND)

install(TARGETS kget_browser_integration DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install(FILES kget_plug_in.rc DESTINATION ${DATA_INSTALL_DIR}/khtml/kpartplugins)
install(FILES kget_plug_in.rc DESTINATION ${DATA_INSTALL_DIR}/dolphinpart/kpartplugins)

if (KWEBKITPART_FOUND)
    install(FILES kget_plug_in.rc DESTINATION ${DATA_INSTALL_DIR}/kwebkitpart/kpartplugins)
endif (KWEBKITPART_FOUND)
