#Set supported mime type
SET(SUPPORTED_KGET_MIMETYPES "${SUPPORTED_KGET_MIMETYPES}application/x-bittorrent;" PARENT_SCOPE)

kde_enable_exceptions()

add_library(kget_bittorrent MODULE)

target_sources(kget_bittorrent PRIVATE
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
    kget_bittorrentfactory.json
    ../../kget_debug.cpp
    )

ki18n_wrap_ui(kget_bittorrent
    btdetailswidgetfrm.ui
    scandlg.ui
    advanceddetails/chunkdownloadview.ui
    advanceddetails/trackerview.ui
    advanceddetails/webseedstab.ui
    )

kconfig_add_kcfg_files(kget_bittorrent bittorrentsettings.kcfgc)

target_link_libraries(kget_bittorrent KF6::KIOCore KTorrent6 kgetcore)
install(TARGETS kget_bittorrent DESTINATION  ${KGET_PLUGIN_INSTALL_DIR})

###Build KCM-Module
kcoreaddons_add_plugin(kcm_kget_bittorrentfactory INSTALL_NAMESPACE "kget_kcms")

target_sources(kcm_kget_bittorrentfactory PRIVATE
    btsettingswidget.cpp
    ../../kget_debug.cpp
    )

ki18n_wrap_ui(kcm_kget_bittorrentfactory
    btsettingswidget.ui
    )

kconfig_add_kcfg_files(kcm_kget_bittorrentfactory bittorrentsettings.kcfgc)
target_link_libraries(kcm_kget_bittorrentfactory KF6::ConfigGui KF6::ConfigWidgets KF6::I18n KF6::KIOCore KF6::KIOWidgets)
