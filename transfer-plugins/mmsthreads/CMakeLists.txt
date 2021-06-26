include_directories(
   ${LIBMMS_INCLUDE_DIR}
)

add_library(kget_mmsfactory MODULE)

target_sources(kget_mmsfactory PRIVATE
  mmsdownload.cpp
  mmstransfer.cpp
  mmstransferfactory.cpp
  mmsthread.cpp
  ../../kget_debug.cpp
)

kconfig_add_kcfg_files(kget_mmsfactory mmssettings.kcfgc)
kcoreaddons_desktop_to_json(kget_mmsfactory kget_mmsfactory.desktop)
target_link_libraries(kget_mmsfactory kgetcore ${LIBMMS_LIBRARIES})

install(TARGETS kget_mmsfactory DESTINATION ${KGET_PLUGIN_INSTALL_DIR})
install(FILES kget_mmsfactory.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})

###Build KCM-Module
add_library(kcm_kget_mmsfactory MODULE)

target_sources(kcm_kget_mmsfactory PRIVATE
  dlgmms.cpp
  ../../kget_debug.cpp
)

ki18n_wrap_ui(kcm_kget_mmsfactory
  dlgmms.ui
)
kconfig_add_kcfg_files(kcm_kget_mmsfactory mmssettings.kcfgc)
target_link_libraries(kcm_kget_mmsfactory KF5::I18n KF5::ConfigWidgets)
install(TARGETS kcm_kget_mmsfactory DESTINATION ${KDE_INSTALL_PLUGINDIR})

install(FILES kget_mmsfactory_config.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
