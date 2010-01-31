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

set(khtml_kget_SRCS
   kget_plug_in.cpp
   links.cpp
)

QT4_ADD_DBUS_INTERFACE(khtml_kget_SRCS ../../dbus/org.kde.kget.main.xml kget_interface )


kde4_add_plugin(khtml_kget ${khtml_kget_SRCS})

target_link_libraries(khtml_kget ${KDE4_KHTML_LIBS})

if(KWEBKITPART_FOUND)
    target_link_libraries(khtml_kget ${KWEBKITPART_LIBRARIES} ${KDE4_KDEUI_LIBRARY} ${QT_QTWEBKIT_LIBRARY})
endif(KWEBKITPART_FOUND)

install(TARGETS khtml_kget DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install(FILES kget_plug_in.rc DESTINATION ${DATA_INSTALL_DIR}/khtml/kpartplugins)

if (KWEBKITPART_FOUND)
    install(FILES kget_plug_in.rc DESTINATION ${DATA_INSTALL_DIR}/kwebkitpart/kpartplugins)
endif (KWEBKITPART_FOUND)
