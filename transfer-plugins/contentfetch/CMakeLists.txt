add_subdirectory(scripts)

include_directories(
   ../../
)

set(kget_contentfetchfactory_PART_SRCS
  contentfetch.cpp
  contentfetchfactory.cpp
  script.cpp
  scriptdownloadengine.cpp
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kget_contentfetchfactory.kcfg.cmake ${CMAKE_CURRENT_SOURCE_DIR}/kget_contentfetchfactory.kcfg)

qt4_wrap_cpp(kget_contentfetchfactory_PART_SRCS scriptconfigadaptor.h)

#kde4_add_ui_files(kget_contentfetchfactory_PART_SRCS dlgcontentfetchsettingwidget.ui dlgscriptediting.ui)

kde4_add_kcfg_files(kget_contentfetchfactory_PART_SRCS contentfetchsetting.kcfgc)

kde4_add_plugin(kget_contentfetchfactory ${kget_contentfetchfactory_PART_SRCS})

target_link_libraries(kget_contentfetchfactory ${KDE4_KROSSUI_LIBS} kgetcore)

install(TARGETS kget_contentfetchfactory DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kget_contentfetchfactory.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES kget_contentfetchfactory.kcfg DESTINATION ${KCFG_INSTALL_DIR})

###Build KCM-Module
set(kcm_kget_contentfetchfactory_PART_SRCS
  dlgcontentfetchsettingwidget.cpp
  dlgscriptediting.cpp
)

qt4_wrap_cpp(kcm_kget_contentfetchfactory_PART_SRCS scriptconfigadaptor.h)

kde4_add_ui_files(kcm_kget_contentfetchfactory_PART_SRCS
  dlgcontentfetchsettingwidget.ui 
  dlgscriptediting.ui
)

kde4_add_kcfg_files(kcm_kget_contentfetchfactory_PART_SRCS contentfetchsetting.kcfgc)
kde4_add_plugin(kcm_kget_contentfetchfactory ${kcm_kget_contentfetchfactory_PART_SRCS})
target_link_libraries(kcm_kget_contentfetchfactory ${KDE4_KDEUI_LIBS} ${KDE4_KROSSUI_LIBS})
install(TARGETS kcm_kget_contentfetchfactory DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES kget_contentfetchfactory_config.desktop DESTINATION ${SERVICES_INSTALL_DIR})
