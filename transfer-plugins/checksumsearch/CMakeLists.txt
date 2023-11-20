add_library(kget_checksumsearchfactory MODULE)

target_sources(kget_checksumsearchfactory PRIVATE
  checksumsearch.cpp
  checksumsearchtransferdatasource.cpp
  checksumsearchfactory.cpp
  kget_checksumsearchfactory.json
  ../../kget_debug.cpp
)

kconfig_add_kcfg_files(kget_checksumsearchfactory checksumsearchsettings.kcfgc)
target_link_libraries(kget_checksumsearchfactory kgetcore)

install(TARGETS kget_checksumsearchfactory DESTINATION  ${KGET_PLUGIN_INSTALL_DIR})
install(FILES kget_checksumsearchfactory.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})

###Build KCM-Module
kcoreaddons_add_plugin(kcm_kget_checksumsearchfactory INSTALL_NAMESPACE "kget_kcms")

target_sources(kcm_kget_checksumsearchfactory PRIVATE
../../kget_debug.cpp
checksumsearch.cpp
dlgchecksumsearch.cpp
)

ki18n_wrap_ui(kcm_kget_checksumsearchfactory
checksumsearch.ui
checksumsearchadddlg.ui
)

kconfig_add_kcfg_files(kcm_kget_checksumsearchfactory checksumsearchsettings.kcfgc)
target_link_libraries(kcm_kget_checksumsearchfactory kgetcore)
