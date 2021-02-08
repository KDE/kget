set(kgetcommonapplet_SRCS
    common/kgetappletutils.cpp
    common/kgetapplet.cpp
)

qt4_add_dbus_interface(kgetcommonapplet_SRCS ../../dbus/org.kde.kget.transfer.xml transfer_interface)

set(kgetbarapplet_SRCS
    barapplet/kgetbarapplet.cpp
    ${kgetcommonapplet_SRCS}
)

qt4_add_dbus_interface(kgetbarapplet_SRCS ../../dbus/org.kde.kget.main.xml kget_interface)

kde4_add_plugin(plasma_kget_barapplet ${kgetbarapplet_SRCS})
target_link_libraries(plasma_kget_barapplet ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS})

install(TARGETS plasma_kget_barapplet DESTINATION ${KDE_INSTALL_PLUGINDIR})


#set(kgetpanelbar_SRCS
#    panelbar/kgetpanelbar.cpp
#    ${kgetcommonapplet_SRCS}
#)
#
#qt4_add_dbus_interface(kgetpanelbar_SRCS ../../../kget/dbus/org.kde.kget.main.xml kget_interface)
#
#kde4_add_plugin(plasma_kget_panelbar ${kgetpanelbar_SRCS})
#target_link_libraries(plasma_kget_panelbar ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS})
#
#install(TARGETS plasma_kget_panelbar DESTINATION ${KDE_INSTALL_PLUGINDIR})

set(kgetpiechart_SRCS
    piechart/kgetpiechart.cpp
    ${kgetcommonapplet_SRCS}
)

qt4_add_dbus_interface(kgetpiechart_SRCS ../../dbus/org.kde.kget.main.xml kget_interface)

kde4_add_plugin(plasma_kget_piechart ${kgetpiechart_SRCS})
target_link_libraries(plasma_kget_piechart ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS})

install(TARGETS plasma_kget_piechart DESTINATION ${KDE_INSTALL_PLUGINDIR})


install(FILES barapplet/kgetbarapplet-default.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
#install(FILES panelbar/kgetpanelbarapplet-default.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install(FILES piechart/kgetpiechartapplet-default.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
