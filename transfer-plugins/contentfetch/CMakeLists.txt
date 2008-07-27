include_directories(
   ../../
)

set(kget_contentfetchfactory_PART_SRCS
  contentfetch.cpp
  contentfetchfactory.cpp
  script.cpp
  scriptdownloadengine.cpp
  dlgcontentfetchsettingwidget.cpp
  dlgscriptediting.cpp
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kget_contentfetchfactory.kcfg.cmake ${CMAKE_CURRENT_SOURCE_DIR}/kget_contentfetchfactory.kcfg)

kde4_add_ui_files(kget_contentfetchfactory_PART_SRCS dlgcontentfetchsettingwidget.ui dlgscriptediting.ui)

kde4_add_kcfg_files(kget_contentfetchfactory_PART_SRCS contentfetchsetting.kcfgc)

kde4_add_plugin(kget_contentfetchfactory ${kget_contentfetchfactory_PART_SRCS})

target_link_libraries(kget_contentfetchfactory ${KDE4_KROSSUI_LIBS} kgetcore)

install(TARGETS kget_contentfetchfactory DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kget_contentfetchfactory.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES kget_contentfetchfactory.kcfg DESTINATION ${KCFG_INSTALL_DIR})
install(FILES scripts/kget_youtubedl.py DESTINATION ${DATA_INSTALL_DIR}/kget/content_fetch_scripts/)