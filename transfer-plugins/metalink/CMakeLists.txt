#Set supported mime type
SET(SUPPORTED_KGET_MIMETYPES "${SUPPORTED_KGET_MIMETYPES}application/metalink+xml;" PARENT_SCOPE)

add_library(kget_metalinkfactory MODULE)

target_sources(kget_metalinkfactory PRIVATE
  metalinkfactory.cpp
  abstractmetalink.cpp
  metalinkxml.cpp
  metalinkhttp.cpp
  fileselectiondlg.cpp
  ../../ui/metalinkcreator/metalinker.cpp
  ../../kget_debug.cpp
)

ki18n_wrap_ui(kget_metalinkfactory
    fileselection.ui
)

kconfig_add_kcfg_files(kget_metalinkfactory metalinksettings.kcfgc)
kcoreaddons_desktop_to_json(kget_metalinkfactory kget_metalinkfactory.desktop)
target_link_libraries(kget_metalinkfactory kgetcore)

install(TARGETS kget_metalinkfactory DESTINATION ${KGET_PLUGIN_INSTALL_DIR})


###Build KCM-Module
add_library(kcm_kget_metalinkfactory MODULE)

target_sources(kcm_kget_metalinkfactory PRIVATE
    dlgmetalink.cpp
    ../../kget_debug.cpp
)

ki18n_wrap_ui(kcm_kget_metalinkfactory
dlgmetalink.ui
)

kconfig_add_kcfg_files(kcm_kget_metalinkfactory metalinksettings.kcfgc)
target_link_libraries(kcm_kget_metalinkfactory KF5::I18n KF5::ConfigGui KF5::ConfigWidgets)
install(TARGETS kcm_kget_metalinkfactory DESTINATION ${KDE_INSTALL_PLUGINDIR})

install(FILES kget_metalinkfactory_config.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
