include_directories(
   ../../
)

set(kget_mirrorsearchfactory_PART_SRCS
  mirrors.cpp
  mirrorsearchtransferdatasource.cpp
  mirrorsearchfactory.cpp
)

kde4_add_kcfg_files(kget_mirrorsearchfactory_PART_SRCS mirrorsearchsettings.kcfgc)

kde4_add_plugin(kget_mirrorsearchfactory ${kget_mirrorsearchfactory_PART_SRCS})

target_link_libraries(kget_mirrorsearchfactory ${KDE4_KIO_LIBS} kgetcore)

install(TARGETS kget_mirrorsearchfactory DESTINATION  ${PLUGIN_INSTALL_DIR})
install(FILES kget_mirrorsearchfactory.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES kget_mirrorsearchfactory.kcfg DESTINATION ${KCFG_INSTALL_DIR})

###Build KCM-Module
set(kcm_kget_mirrorsearchfactory_PART_SRCS
  dlgmirrorsearch.cpp
)

kde4_add_ui_files(kcm_kget_mirrorsearchfactory_PART_SRCS
  dlgmirrorsearch.ui 
  dlgengineediting.ui
)

kde4_add_kcfg_files(kcm_kget_mirrorsearchfactory_PART_SRCS mirrorsearchsettings.kcfgc)
kde4_add_plugin(kcm_kget_mirrorsearchfactory ${kcm_kget_mirrorsearchfactory_PART_SRCS})
target_link_libraries(kcm_kget_mirrorsearchfactory ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS})
install(TARGETS kcm_kget_mirrorsearchfactory DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES kget_mirrorsearchfactory_config.desktop DESTINATION ${SERVICES_INSTALL_DIR})
