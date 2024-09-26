add_library(kget_mirrorsearchfactory MODULE)

target_sources(kget_mirrorsearchfactory PRIVATE
  mirrors.cpp
  mirrorsearchtransferdatasource.cpp
  mirrorsearchfactory.cpp
  kget_mirrorsearchfactory.json
  ${kget_debug_SRCS}
)

kconfig_add_kcfg_files(kget_mirrorsearchfactory mirrorsearchsettings.kcfgc)

target_link_libraries(kget_mirrorsearchfactory kgetcore)

install(TARGETS kget_mirrorsearchfactory DESTINATION  ${KGET_PLUGIN_INSTALL_DIR})
install(FILES kget_mirrorsearchfactory.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})

###Build KCM-Module
kcoreaddons_add_plugin(kcm_kget_mirrorsearchfactory INSTALL_NAMESPACE "kget_kcms")

target_sources(kcm_kget_mirrorsearchfactory PRIVATE
  dlgmirrorsearch.cpp
  ${kget_debug_SRCS}
)

ki18n_wrap_ui(kcm_kget_mirrorsearchfactory
  dlgmirrorsearch.ui 
  dlgengineediting.ui
)

kconfig_add_kcfg_files(kcm_kget_mirrorsearchfactory mirrorsearchsettings.kcfgc)
target_link_libraries(kcm_kget_mirrorsearchfactory KF6::ConfigWidgets KF6::CoreAddons KF6::I18n KF6::WidgetsAddons KF6::KCMUtils)
