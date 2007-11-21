
find_package(BTCore REQUIRED)
if(BTCORE_FOUND)
include_directories(
   ../../
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_BINARY_DIR}
   ${KDE4_INCLUDES}
   ${BTCORE_INCLUDE_DIR}
)
add_definitions(${KDE4_ENABLE_EXCEPTIONS} ${KDE4_DEFINITIONS} )
########### next target ###############

set(kget_bittorrentfactory_PART_SRCS
  bttransfer.cpp
  bttransferfactory.cpp
  bttransferhandler.cpp
  btdetailswidget.cpp
)

kde4_add_ui_files(kget_bittorrentfactory_PART_SRCS btdetailswidgetfrm.ui)

#kde4_add_kcfg_files(kget_bittorrentfactory_PART_SRCS bittorrentsettings.kcfgc)

kde4_add_plugin(kget_bittorrentfactory WITH_PREFIX ${kget_bittorrentfactory_PART_SRCS})

target_link_libraries(kget_bittorrentfactory ${KDE4_KDEUI_LIBS} ${BTCORE_LIBRARIES}  kgetcore)

install(TARGETS kget_bittorrentfactory DESTINATION  ${PLUGIN_INSTALL_DIR})

########### install files ###############

install(FILES kget_bittorrentfactory.desktop DESTINATION ${SERVICES_INSTALL_DIR})
#install(FILES kget_bittorrentfactory.kcfg DESTINATION ${KCFG_INSTALL_DIR})

endif(BTCORE_FOUND)
