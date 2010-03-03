macro_optional_find_package(GMP)
macro_log_feature(GMP_FOUND "GMP" "The GNU Multiple Precision Arithmetic Library" "http://gmplib.org/" FALSE "" "Needed to build KGet Torrent support.")

if (GMP_FOUND AND QCA2_FOUND)
    find_package(BTCore)
    macro_log_feature(BTCORE_FOUND "BTCore" "KTorrent core library" "http://ktorrent.org/" FALSE "" "Needed to build Torrent support in KGet.")

    include_directories(
        ../../
       ${BTCORE_INCLUDE_DIR}
    )
endif (GMP_FOUND AND QCA2_FOUND)

if (BTCORE_FOUND)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

   set(kget_bittorrentfactory_PART_SRCS
      bttransfer.cpp
      bttransferfactory.cpp
      bttransferhandler.cpp
      btdetailswidget.cpp
      scandlg.cpp
      #btdatasource.cpp
      #btchunkselector.cpp
      #btcache.cpp
      advanceddetails/btadvanceddetailswidget.cpp
      advanceddetails/chunkdownloadmodel.cpp
      advanceddetails/chunkdownloadview.cpp
      advanceddetails/fileview.cpp
      advanceddetails/iwfilelistmodel.cpp
      advanceddetails/iwfiletreemodel.cpp
      advanceddetails/peerview.cpp
      advanceddetails/peerviewmodel.cpp
      advanceddetails/torrentfilelistmodel.cpp
      advanceddetails/torrentfilemodel.cpp
      advanceddetails/torrentfiletreemodel.cpp
      advanceddetails/monitor.cpp
      advanceddetails/trackerview.cpp
      advanceddetails/trackermodel.cpp
      advanceddetails/webseedsmodel.cpp
      advanceddetails/webseedstab.cpp
      )

   if(HAVE_NEPOMUK)
      set(kget_bittorrentfactory_PART_SRCS
         ${kget_bittorrentfactory_PART_SRCS}
         btnepomukhandler.cpp
         )
   endif(HAVE_NEPOMUK)

   kde4_add_ui_files(kget_bittorrentfactory_PART_SRCS
      btdetailswidgetfrm.ui
      scandlg.ui
      advanceddetails/chunkdownloadview.ui
      advanceddetails/trackerview.ui
      advanceddetails/webseedstab.ui
      )

   kde4_add_kcfg_files(kget_bittorrentfactory_PART_SRCS bittorrentsettings.kcfgc)
   kde4_add_plugin(kget_bittorrentfactory ${kget_bittorrentfactory_PART_SRCS})
   if (HAVE_NEPOMUK)
      target_link_libraries(kget_bittorrentfactory ${SOPRANO_LIBRARIES} ${NEPOMUK_LIBRARIES})
   endif (HAVE_NEPOMUK)
   target_link_libraries(kget_bittorrentfactory ${KDE4_KIO_LIBS} ${BTCORE_LIBRARIES} kgetcore)
   install(TARGETS kget_bittorrentfactory DESTINATION  ${PLUGIN_INSTALL_DIR})
   install(FILES kget_bittorrentfactory.desktop DESTINATION ${SERVICES_INSTALL_DIR})

   ###Build KCM-Module
   set(kcm_kget_bittorrentfactory_PART_SRCS
      btsettingswidget.cpp
      )

   kde4_add_ui_files(kcm_kget_bittorrentfactory_PART_SRCS
      btsettingswidget.ui
      )

   kde4_add_kcfg_files(kcm_kget_bittorrentfactory_PART_SRCS bittorrentsettings.kcfgc)
   kde4_add_plugin(kcm_kget_bittorrentfactory ${kcm_kget_bittorrentfactory_PART_SRCS})
   target_link_libraries(kcm_kget_bittorrentfactory ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS})
   install(TARGETS kcm_kget_bittorrentfactory DESTINATION ${PLUGIN_INSTALL_DIR})

   install(FILES kget_bittorrentfactory_config.desktop DESTINATION ${SERVICES_INSTALL_DIR})
endif (BTCORE_FOUND)

