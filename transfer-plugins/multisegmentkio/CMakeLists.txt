include_directories(
   ../../
)

set(kget_multisegkiofactory_PART_SRCS
  dlgmultisegkio.cpp
  segmentfactory.cpp
  multisegkio.cpp
  mirrors.cpp
  transfermultisegkio.cpp
  transfermultisegkiofactory.cpp
)

kde4_add_ui_files(kget_multisegkiofactory_PART_SRCS dlgmultisegkio.ui dlgengineediting.ui)

kde4_add_kcfg_files(kget_multisegkiofactory_PART_SRCS multisegkiosettings.kcfgc)

kde4_add_plugin(kget_multisegkiofactory ${kget_multisegkiofactory_PART_SRCS})

target_link_libraries(kget_multisegkiofactory ${KDE4_KIO_LIBS} kgetcore)

install(TARGETS kget_multisegkiofactory DESTINATION  ${PLUGIN_INSTALL_DIR})
install(FILES kget_multisegkiofactory.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES kget_multisegkiofactory.kcfg DESTINATION ${KCFG_INSTALL_DIR})
