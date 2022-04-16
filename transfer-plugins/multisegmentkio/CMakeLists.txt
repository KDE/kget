add_library(kget_multisegkiofactory MODULE)

target_sources(kget_multisegkiofactory PRIVATE
  segment.cpp
  multisegkiodatasource.cpp
  transfermultisegkio.cpp
  transfermultisegkiofactory.cpp
  ../../kget_debug.cpp
)

kconfig_add_kcfg_files(kget_multisegkiofactory multisegkiosettings.kcfgc)

kcoreaddons_desktop_to_json(kget_multisegkiofactory kget_multisegkiofactory.desktop)
target_link_libraries(kget_multisegkiofactory kgetcore)

install(TARGETS kget_multisegkiofactory DESTINATION ${KGET_PLUGIN_INSTALL_DIR})
install(FILES kget_multisegkiofactory.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})

###Build KCM-Module
kcoreaddons_add_plugin(kcm_kget_multisegkiofactory INSTALL_NAMESPACE "kget_kcms")

target_sources(kcm_kget_multisegkiofactory PRIVATE
  dlgmultisegkio.cpp
  ../../kget_debug.cpp
)

ki18n_wrap_ui(kcm_kget_multisegkiofactory
  dlgmultisegkio.ui
)

kconfig_add_kcfg_files(kcm_kget_multisegkiofactory multisegkiosettings.kcfgc)
target_link_libraries(kcm_kget_multisegkiofactory KF5::I18n KF5::ConfigWidgets KF5::CoreAddons)
