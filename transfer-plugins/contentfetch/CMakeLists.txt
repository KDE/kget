add_subdirectory(scripts)

add_library(kget_contentfetchfactory MODULE)

target_sources(kget_contentfetchfactory PRIVATE
  contentfetch.cpp
  contentfetchfactory.cpp
  script.cpp
  scriptdownloadengine.cpp
  scriptconfigadaptor.cpp
)

kconfig_add_kcfg_files(kget_contentfetchfactory contentfetchsetting.kcfgc)
kcoreaddons_desktop_to_json(kget_contentfetchfactory kget_contentfetchfactory.desktop)
target_link_libraries(kget_contentfetchfactory KF5::KIOCore KF5::KrossCore kgetcore)

install(TARGETS kget_contentfetchfactory DESTINATION ${KGET_PLUGIN_INSTALL_DIR})
install(FILES kget_contentfetchfactory.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})

###Build KCM-Module
kcoreaddons_add_plugin(kcm_kget_contentfetchfactory INSTALL_NAMESPACE "kget_kcms")

target_sources(kcm_kget_contentfetchfactory PRIVATE
  dlgcontentfetchsettingwidget.cpp
  dlgscriptediting.cpp
  scriptconfigadaptor.cpp
)

ki18n_wrap_ui(kcm_kget_contentfetchfactory
  dlgcontentfetchsettingwidget.ui 
  dlgscriptediting.ui
)

kconfig_add_kcfg_files(kcm_kget_contentfetchfactory contentfetchsetting.kcfgc)
target_link_libraries(kcm_kget_contentfetchfactory KF5::KIOCore
KF5::KrossCore ${KDE4_KROSSUI_LIBS})
