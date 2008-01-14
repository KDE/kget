include_directories(
   ../../
)

set(kget_mirrorsearchfactory_PART_SRCS
  dlgmirrorsearch.cpp
  mirrors.cpp
  mirrorsearchtransferdatasource.cpp
  mirrorsearchfactory.cpp
)

kde4_add_ui_files(kget_mirrorsearchfactory_PART_SRCS dlgmirrorsearch.ui dlgengineediting.ui)

kde4_add_kcfg_files(kget_mirrorsearchfactory_PART_SRCS mirrorsearchsettings.kcfgc)

kde4_add_plugin(kget_mirrorsearchfactory ${kget_mirrorsearchfactory_PART_SRCS})

target_link_libraries(kget_mirrorsearchfactory ${KDE4_KIO_LIBS} kgetcore)

install(TARGETS kget_mirrorsearchfactory DESTINATION  ${PLUGIN_INSTALL_DIR})
install(FILES kget_mirrorsearchfactory.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES kget_mirrorsearchfactory.kcfg DESTINATION ${KCFG_INSTALL_DIR})
