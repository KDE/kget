include_directories(
   ../../
   ${LIBMMS_INCLUDE_DIR}
)

set(kget_mmsfactory_PART_SRCS
  mmsdownload.cpp
  mmstransfer.cpp
  mmstransferfactory.cpp
  mmsthread.cpp
  ../../kget_debug.cpp
)

kconfig_add_kcfg_files(kget_mmsfactory_PART_SRCS mmssettings.kcfgc)
add_library(kget_mmsfactory MODULE ${kget_mmsfactory_PART_SRCS})
kcoreaddons_desktop_to_json(kget_mmsfactory kget_mmsfactory.desktop)
target_link_libraries(kget_mmsfactory kgetcore ${LIBMMS_LIBRARIES})

install(TARGETS kget_mmsfactory DESTINATION ${KGET_PLUGIN_INSTALL_DIR})
install(FILES kget_mmsfactory.kcfg DESTINATION ${KCFG_INSTALL_DIR})

###Build KCM-Module
set(kcm_kget_mmsfactory_PART_SRCS
  dlgmms.cpp
  ../../kget_debug.cpp
)

ki18n_wrap_ui(kcm_kget_mmsfactory_PART_SRCS
  dlgmms.ui
)
kconfig_add_kcfg_files(kcm_kget_mmsfactory_PART_SRCS mmssettings.kcfgc)
add_library(kcm_kget_mmsfactory MODULE ${kcm_kget_mmsfactory_PART_SRCS})
target_link_libraries(kcm_kget_mmsfactory ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS})
install(TARGETS kcm_kget_mmsfactory DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES kget_mmsfactory_config.desktop DESTINATION ${SERVICES_INSTALL_DIR})
