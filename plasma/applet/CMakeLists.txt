set(kgetbarapplet_SRCS
    barapplet/kgetbarapplet.cpp
    common/kgetappletutils.cpp
)

kde4_add_plugin(plasma_kget_barapplet ${kgetbarapplet_SRCS})
target_link_libraries(plasma_kget_barapplet ${PLASMA_LIBS} ${KDE4_KDEUI_LIBS})

install(TARGETS plasma_kget_barapplet DESTINATION ${PLUGIN_INSTALL_DIR})


set(kgetpanelbar_SRCS
    panelbar/kgetpanelbar.cpp
    common/kgetappletutils.cpp
)

kde4_add_plugin(plasma_kget_panelbar ${kgetpanelbar_SRCS})
target_link_libraries(plasma_kget_panelbar ${PLASMA_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS plasma_kget_panelbar DESTINATION ${PLUGIN_INSTALL_DIR})

set(kgetpiechart_SRCS
    piechart/kgetpiechart.cpp
    common/kgetappletutils.cpp
)

kde4_add_plugin(plasma_kget_piechart ${kgetpiechart_SRCS})
target_link_libraries(plasma_kget_piechart ${PLASMA_LIBS} ${KDE4_KDEUI_LIBS})

install(TARGETS plasma_kget_piechart DESTINATION ${PLUGIN_INSTALL_DIR})


install(FILES barapplet/kgetbarapplet-default.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES panelbar/kgetpanelbarapplet-default.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES piechart/kgetpiechartapplet-default.desktop DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES common/kget.svg DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/widgets/)
