#Set supported mime type
SET(SUPPORTED_KGET_MIMETYPES "${SUPPORTED_KGET_MIMETYPES}application/metalink+xml;" PARENT_SCOPE)

add_library(kget_metalinkfactory MODULE)

target_sources(kget_metalinkfactory PRIVATE
  metalinkfactory.cpp
  abstractmetalink.cpp
  metalinkxml.cpp
  metalinkhttp.cpp
  fileselectiondlg.cpp
  kget_metalinkfactory.json
  ../../ui/metalinkcreator/metalinker.cpp
  ../../kget_debug.cpp
)

ki18n_wrap_ui(kget_metalinkfactory
    fileselection.ui
)

kconfig_add_kcfg_files(kget_metalinkfactory metalinksettings.kcfgc)
target_link_libraries(kget_metalinkfactory kgetcore)

install(TARGETS kget_metalinkfactory DESTINATION ${KGET_PLUGIN_INSTALL_DIR})


###Build KCM-Module
kcoreaddons_add_plugin(kcm_kget_metalinkfactory INSTALL_NAMESPACE "kget_kcms")

target_sources(kcm_kget_metalinkfactory PRIVATE
    dlgmetalink.cpp
    ../../kget_debug.cpp
)

ki18n_wrap_ui(kcm_kget_metalinkfactory
dlgmetalink.ui
)

kconfig_add_kcfg_files(kcm_kget_metalinkfactory metalinksettings.kcfgc)
target_link_libraries(kcm_kget_metalinkfactory KF6::I18n KF6::ConfigGui KF6::CoreAddons KF6::ConfigWidgets KF6::KCMUtils)
