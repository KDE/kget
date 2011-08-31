include_directories(
   ../../
   ${LIBMMS_INCLUDE_DIR}
)

set(kget_mmsfactory_PART_SRCS
  mmsdownload.cpp
  mmstransfer.cpp
  mmstransferfactory.cpp
  mmsthread.cpp
)

kde4_add_kcfg_files(kget_mmsfactory_PART_SRCS mmssettings.kcfgc)
kde4_add_plugin(kget_mmsfactory ${kget_mmsfactory_PART_SRCS})

target_link_libraries(kget_mmsfactory ${KDE4_KIO_LIBS} kgetcore ${LIBMMS_LIBRARIES})

install(TARGETS kget_mmsfactory DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kget_mmsfactory.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES kget_mmsfactory.kcfg DESTINATION ${KCFG_INSTALL_DIR})

###Build KCM-Module
set(kcm_kget_mmsfactory_PART_SRCS
  dlgmms.cpp
)

kde4_add_ui_files(kcm_kget_mmsfactory_PART_SRCS
  dlgmms.ui
)
kde4_add_kcfg_files(kcm_kget_mmsfactory_PART_SRCS mmssettings.kcfgc)
kde4_add_plugin(kcm_kget_mmsfactory ${kcm_kget_mmsfactory_PART_SRCS})
target_link_libraries(kcm_kget_mmsfactory ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS})
install(TARGETS kcm_kget_mmsfactory DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES kget_mmsfactory_config.desktop DESTINATION ${SERVICES_INSTALL_DIR})
