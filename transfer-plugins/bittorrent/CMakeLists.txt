macro_optional_find_package(BTCore)
macro_log_feature(BTCORE_FOUND "BTCore" "KTorrent core library" "http://ktorrent.org/" FALSE "" "Needed to build Torrent support in KGet")

if (BTCORE_FOUND)
add_definitions(${KDE4_ENABLE_EXCEPTIONS})

include_directories(
   ../../
   ${BTCORE_INCLUDE_DIR}
)

set(kget_bittorrentfactory_PART_SRCS
  bttransfer.cpp
  bttransferfactory.cpp
  bttransferhandler.cpp
  btdetailswidget.cpp
  btsettingswidget.cpp
  btadvanceddetailswidget.cpp
  btdownload.cpp
  peerview.cpp
  torrentfiletreemodel.cpp
)

kde4_add_ui_files(kget_bittorrentfactory_PART_SRCS btdetailswidgetfrm.ui btsettingswidget.ui btadvanceddetailswidget.ui)

kde4_add_kcfg_files(kget_bittorrentfactory_PART_SRCS bittorrentsettings.kcfgc)

kde4_add_plugin(kget_bittorrentfactory ${kget_bittorrentfactory_PART_SRCS})

target_link_libraries(kget_bittorrentfactory ${KDE4_KDEUI_LIBS} ${BTCORE_LIBRARIES} kgetcore)

install(TARGETS kget_bittorrentfactory DESTINATION  ${PLUGIN_INSTALL_DIR})
install(FILES kget_bittorrentfactory.desktop DESTINATION ${SERVICES_INSTALL_DIR})

endif(BTCORE_FOUND)
