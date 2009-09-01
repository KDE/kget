include_directories(
   ../../
)

set(kget_multisegkiofactory_PART_SRCS
  segment.cpp
  multisegkiodatasource.cpp
  transfermultisegkio.cpp
  transfermultisegkiofactory.cpp
)

kde4_add_kcfg_files(kget_multisegkiofactory_PART_SRCS multisegkiosettings.kcfgc)

kde4_add_plugin(kget_multisegkiofactory ${kget_multisegkiofactory_PART_SRCS})

target_link_libraries(kget_multisegkiofactory ${KDE4_KIO_LIBS} kgetcore)

install(TARGETS kget_multisegkiofactory DESTINATION  ${PLUGIN_INSTALL_DIR})
install(FILES kget_multisegkiofactory.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES kget_multisegkiofactory.kcfg DESTINATION ${KCFG_INSTALL_DIR})

###Build KCM-Module
set(kcm_kget_multisegkiofactory_PART_SRCS
  dlgmultisegkio.cpp
)

kde4_add_ui_files(kcm_kget_multisegkiofactory_PART_SRCS
  dlgmultisegkio.ui
)

kde4_add_kcfg_files(kcm_kget_multisegkiofactory_PART_SRCS multisegkiosettings.kcfgc)
kde4_add_plugin(kcm_kget_multisegkiofactory ${kcm_kget_multisegkiofactory_PART_SRCS})
target_link_libraries(kcm_kget_multisegkiofactory ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS})
install(TARGETS kcm_kget_multisegkiofactory DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES kget_multisegkiofactory_config.desktop DESTINATION ${SERVICES_INSTALL_DIR})
