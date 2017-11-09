include_directories(
   ../../
)

set(kget_mirrorsearchfactory_PART_SRCS
  mirrors.cpp
  mirrorsearchtransferdatasource.cpp
  mirrorsearchfactory.cpp
  ../../kget_debug.cpp
)

kconfig_add_kcfg_files(kget_mirrorsearchfactory_PART_SRCS mirrorsearchsettings.kcfgc)

add_library(kget_mirrorsearchfactory MODULE ${kget_mirrorsearchfactory_PART_SRCS})
kcoreaddons_desktop_to_json(kget_mirrorsearchfactory kget_mirrorsearchfactory.desktop)
target_link_libraries(kget_mirrorsearchfactory ${KDE4_KIO_LIBS} kgetcore)

install(TARGETS kget_mirrorsearchfactory DESTINATION  ${KGET_PLUGIN_INSTALL_DIR})
install(FILES kget_mirrorsearchfactory.kcfg DESTINATION ${KCFG_INSTALL_DIR})

###Build KCM-Module
set(kcm_kget_mirrorsearchfactory_PART_SRCS
  dlgmirrorsearch.cpp
  ../../kget_debug.cpp
)

ki18n_wrap_ui(kcm_kget_mirrorsearchfactory_PART_SRCS
  dlgmirrorsearch.ui 
  dlgengineediting.ui
)

kconfig_add_kcfg_files(kcm_kget_mirrorsearchfactory_PART_SRCS mirrorsearchsettings.kcfgc)
add_library(kcm_kget_mirrorsearchfactory MODULE ${kcm_kget_mirrorsearchfactory_PART_SRCS})
target_link_libraries(kcm_kget_mirrorsearchfactory KF5::I18n KF5::KDELibs4Support)
install(TARGETS kcm_kget_mirrorsearchfactory DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES kget_mirrorsearchfactory_config.desktop DESTINATION ${SERVICES_INSTALL_DIR})
