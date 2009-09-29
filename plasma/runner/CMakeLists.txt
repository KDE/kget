set(krunner_kget_SRCS
    kgetrunner.cpp
    )
kde4_add_plugin(krunner_kget
    ${krunner_kget_SRCS}
    )
target_link_libraries(krunner_kget
    ${KDE4_PLASMA_LIBS}
    )


set(kcm_krunner_kget_SRCS
    kgetrunner_config.cpp
)
kde4_add_ui_files(kcm_krunner_kget_SRCS
    kgetrunner_config.ui
    )
kde4_add_plugin(kcm_krunner_kget
    ${kcm_krunner_kget_SRCS}
    )
target_link_libraries(kcm_krunner_kget
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KUTILS_LIBS}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    )

install(TARGETS
    krunner_kget
    kcm_krunner_kget
    DESTINATION ${PLUGIN_INSTALL_DIR}
    )
install(FILES
    plasma-runner-kget.desktop
    plasma-runner-kget_config.desktop
    DESTINATION ${SERVICES_INSTALL_DIR}
    )
