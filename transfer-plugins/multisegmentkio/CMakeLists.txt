include_directories(
   ../../
)

set(kget_multisegkiofactory_PART_SRCS
  segment.cpp
  multisegkiodatasource.cpp
  transfermultisegkio.cpp
  transfermultisegkiofactory.cpp
  ../../kget_debug.cpp
)

kconfig_add_kcfg_files(kget_multisegkiofactory_PART_SRCS multisegkiosettings.kcfgc)

add_library(kget_multisegkiofactory MODULE ${kget_multisegkiofactory_PART_SRCS})
kcoreaddons_desktop_to_json(kget_multisegkiofactory kget_multisegkiofactory.desktop)
target_link_libraries(kget_multisegkiofactory kgetcore)

install(TARGETS kget_multisegkiofactory DESTINATION ${KGET_PLUGIN_INSTALL_DIR})
install(FILES kget_multisegkiofactory.kcfg DESTINATION ${KCFG_INSTALL_DIR})

###Build KCM-Module
set(kcm_kget_multisegkiofactory_PART_SRCS
  dlgmultisegkio.cpp
  ../../kget_debug.cpp
)

ki18n_wrap_ui(kcm_kget_multisegkiofactory_PART_SRCS
  dlgmultisegkio.ui
)

kconfig_add_kcfg_files(kcm_kget_multisegkiofactory_PART_SRCS multisegkiosettings.kcfgc)
add_library(kcm_kget_multisegkiofactory MODULE ${kcm_kget_multisegkiofactory_PART_SRCS})
target_link_libraries(kcm_kget_multisegkiofactory KF5::I18n KF5::ConfigWidgets)
install(TARGETS kcm_kget_multisegkiofactory DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES kget_multisegkiofactory_config.desktop DESTINATION ${SERVICES_INSTALL_DIR})
