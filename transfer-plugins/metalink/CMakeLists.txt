#Set supported mime type
SET(SUPPORTED_KGET_MIMETYPES "${SUPPORTED_KGET_MIMETYPES}application/metalink+xml;" PARENT_SCOPE)

include_directories(
   ../../
)

set(kget_metalinkfactory_PART_SRCS
  metalinkfactory.cpp
  abstractmetalink.cpp
  metalinkxml.cpp
  metalinkhttp.cpp
  fileselectiondlg.cpp
  ../../ui/metalinkcreator/metalinker.cpp
  ../../kget_debug.cpp
)

ki18n_wrap_ui(kget_metalinkfactory_PART_SRCS
    fileselection.ui
)

kconfig_add_kcfg_files(kget_metalinkfactory_PART_SRCS metalinksettings.kcfgc)
add_library(kget_metalinkfactory MODULE ${kget_metalinkfactory_PART_SRCS})
kcoreaddons_desktop_to_json(kget_metalinkfactory kget_metalinkfactory.desktop)
target_link_libraries(kget_metalinkfactory kgetcore)

install(TARGETS kget_metalinkfactory DESTINATION ${KGET_PLUGIN_INSTALL_DIR})


###Build KCM-Module
set(kcm_kget_metalinkfactory_PART_SRCS
    dlgmetalink.cpp
    ../../kget_debug.cpp
)

ki18n_wrap_ui(kcm_kget_metalinkfactory_PART_SRCS
dlgmetalink.ui
)

kconfig_add_kcfg_files(kcm_kget_metalinkfactory_PART_SRCS metalinksettings.kcfgc)
add_library(kcm_kget_metalinkfactory MODULE ${kcm_kget_metalinkfactory_PART_SRCS})
target_link_libraries(kcm_kget_metalinkfactory KF5::I18n KF5::ConfigGui KF5::ConfigWidgets)
install(TARGETS kcm_kget_metalinkfactory DESTINATION ${KDE_INSTALL_PLUGINDIR})

install(FILES kget_metalinkfactory_config.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
