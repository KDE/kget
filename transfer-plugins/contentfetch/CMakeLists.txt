add_subdirectory(scripts)

include_directories(
   ../../
)

set(kget_contentfetchfactory_PART_SRCS
  contentfetch.cpp
  contentfetchfactory.cpp
  script.cpp
  scriptdownloadengine.cpp
  scriptconfigadaptor.cpp
)

kconfig_add_kcfg_files(kget_contentfetchfactory_PART_SRCS contentfetchsetting.kcfgc)
kcoreaddons_desktop_to_json(kget_contentfetchfactory kget_contentfetchfactory.desktop)
add_library(kget_contentfetchfactory MODULE ${kget_contentfetchfactory_PART_SRCS})
target_link_libraries(kget_contentfetchfactory KF5::KIOCore KF5::KrossCore kgetcore)

install(TARGETS kget_contentfetchfactory DESTINATION ${KGET_PLUGIN_INSTALL_DIR})
install(FILES kget_contentfetchfactory.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})

###Build KCM-Module
set(kcm_kget_contentfetchfactory_PART_SRCS
  dlgcontentfetchsettingwidget.cpp
  dlgscriptediting.cpp
  scriptconfigadaptor.cpp
)

ki18n_wrap_ui(kcm_kget_contentfetchfactory_PART_SRCS
  dlgcontentfetchsettingwidget.ui 
  dlgscriptediting.ui
)

kconfig_add_kcfg_files(kcm_kget_contentfetchfactory_PART_SRCS contentfetchsetting.kcfgc)
add_library(kcm_kget_contentfetchfactory MODULE ${kcm_kget_contentfetchfactory_PART_SRCS})
target_link_libraries(kcm_kget_contentfetchfactory KF5::KIOCore
KF5::KrossCore ${KDE4_KROSSUI_LIBS})
install(TARGETS kcm_kget_contentfetchfactory DESTINATION ${KDE_INSTALL_PLUGINDIR})

install(FILES kget_contentfetchfactory_config.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
