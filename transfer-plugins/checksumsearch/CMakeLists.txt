add_library(kget_checksumsearchfactory MODULE)

target_sources(kget_checksumsearchfactory PRIVATE
  checksumsearch.cpp
  checksumsearchtransferdatasource.cpp
  checksumsearchfactory.cpp
  ../../kget_debug.cpp
)

kconfig_add_kcfg_files(kget_checksumsearchfactory checksumsearchsettings.kcfgc)
kcoreaddons_desktop_to_json(kget_checksumsearchfactory kget_checksumsearchfactory.desktop)
target_link_libraries(kget_checksumsearchfactory kgetcore)

install(TARGETS kget_checksumsearchfactory DESTINATION  ${KGET_PLUGIN_INSTALL_DIR})
install(FILES kget_checksumsearchfactory.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})

###Build KCM-Module
add_library(kcm_kget_checksumsearchfactory MODULE)

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
install(TARGETS kcm_kget_checksumsearchfactory DESTINATION ${KDE_INSTALL_PLUGINDIR})

install(FILES kget_checksumsearchfactory_config.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
