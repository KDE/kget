option(ENABLE_EMBEDDED_TORRENT_SUPPORT "Whether to build KGet torrent support with embedded torrent library or not" true)

macro_optional_find_package(GMP)
macro_log_feature(GMP_FOUND "GMP" "The GNU Multiple Precision Arithmetic Library" "http://gmplib.org/" FALSE "" "Needed to build KGet Torrent support.")

if (ENABLE_EMBEDDED_TORRENT_SUPPORT AND GMP_FOUND AND QCA2_FOUND)
    message(STATUS "Building KGet torrent support with embedded torrent library (libbtcore) (-DENABLE_EMBEDDED_TORRENT_SUPPORT=false to disable)")
    add_definitions(-DENABLE_EMBEDDED_TORRENT_SUPPORT)

include_directories(
   ../../
   libbtcore
   ${QCA2_INCLUDE_DIR}
   ${GMP_INCLUDE_DIR}
)

# keep the following list in sync with libbtcore/CMakeLists.txt
#XFS prealloc
CHECK_INCLUDE_FILES(xfs/xfs.h HAVE_XFS_XFS_H)
IF(HAVE_XFS_XFS_H)
       SET(CMAKE_EXTRA_INCLUDE_FILES xfs/xfs.h)
       CHECK_TYPE_SIZE(__u64 HAVE___U64)
       CHECK_TYPE_SIZE(__s64 HAVE___S64)
       SET(CMAKE_EXTRA_INCLUDE_FILES)
ENDIF(HAVE_XFS_XFS_H)

# check for 64 bit file I/O functions
check_function_exists(fopen64 HAVE_FOPEN64)
check_function_exists(fseeko64 HAVE_FSEEKO64)
check_function_exists(fseeko HAVE_FSEEKO)
check_function_exists(ftello64 HAVE_FTELLO64)
check_function_exists(ftello HAVE_FTELLO)
check_function_exists(fstat64 HAVE_FSTAT64)
check_function_exists(stat64 HAVE_STAT64)
check_function_exists(ftruncate64 HAVE_FTRUNCATE64)
check_function_exists(lseek64 HAVE_LSEEK64)
check_function_exists(mmap64 HAVE_MMAP64)
check_function_exists(munmap64 HAVE_MUNMAP64)
check_function_exists(posix_fallocate64 HAVE_POSIX_FALLOCATE64)
check_function_exists(posix_fallocate HAVE_POSIX_FALLOCATE)
check_function_exists(statvfs HAVE_STATVFS)
check_function_exists(statvfs64 HAVE_STATVFS64)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/libbtcore/config-btcore.h.cmake ${CMAKE_BINARY_DIR}/config-btcore.h)

set(libbtcore_SRC
    libbtcore/util/bitset.cpp
    libbtcore/util/timer.cpp
    libbtcore/util/urlencoder.cpp
    libbtcore/util/sha1hashgen.cpp
    libbtcore/util/sha1hash.cpp
    libbtcore/util/functions.cpp
    libbtcore/util/ptrmap.cpp
    libbtcore/util/array.cpp
    libbtcore/util/error.cpp
    libbtcore/util/file.cpp
    libbtcore/util/fileops.cpp
    libbtcore/util/log.cpp
    libbtcore/util/waitjob.cpp
    libbtcore/util/autorotatelogjob.cpp
    libbtcore/util/compressfilejob.cpp
    libbtcore/util/logsystemmanager.cpp

    libbtcore/bcodec/bdecoder.cpp  
    libbtcore/bcodec/bencoder.cpp  
    libbtcore/bcodec/bnode.cpp  
    libbtcore/bcodec/value.cpp

    libbtcore/net/address.cpp  
    libbtcore/net/bufferedsocket.cpp  
    libbtcore/net/portlist.cpp  
    libbtcore/net/socket.cpp  
    libbtcore/net/socketmonitor.cpp  
    libbtcore/net/speed.cpp
    libbtcore/net/uploadthread.cpp
    libbtcore/net/downloadthread.cpp
    libbtcore/net/networkthread.cpp
    libbtcore/net/socketgroup.cpp
    libbtcore/net/socks.cpp
    libbtcore/net/wakeuppipe.cpp
    libbtcore/net/reverseresolver.cpp
    
    libbtcore/mse/bigint.cpp  
    libbtcore/mse/functions.cpp  
    libbtcore/mse/rc4encryptor.cpp  
    libbtcore/mse/streamsocket.cpp
    libbtcore/mse/encryptedauthenticate.cpp
    libbtcore/mse/encryptedserverauthenticate.cpp
    
    libbtcore/peer/authenticatebase.cpp
    libbtcore/peer/authenticate.cpp
    libbtcore/peer/serverauthenticate.cpp
    libbtcore/peer/authenticationmonitor.cpp
    libbtcore/peer/chunkcounter.cpp
    libbtcore/peer/peerid.cpp
    libbtcore/peer/peer.cpp
    libbtcore/peer/peermanager.cpp
    libbtcore/peer/peerdownloader.cpp
    libbtcore/peer/peeruploader.cpp
    libbtcore/peer/packetwriter.cpp
    libbtcore/peer/packetreader.cpp
    libbtcore/peer/utpex.cpp
    libbtcore/peer/accessmanager.cpp
    libbtcore/peer/badpeerslist.cpp

    libbtcore/download/piece.cpp
    libbtcore/download/request.cpp
    libbtcore/download/packet.cpp
    libbtcore/download/webseed.cpp
    libbtcore/download/chunkdownload.cpp
    libbtcore/download/chunkselector.cpp
    libbtcore/download/downloader.cpp
    libbtcore/download/httpconnection.cpp

    libbtcore/interfaces/piecedownloader.cpp
    libbtcore/interfaces/peerinterface.cpp
    libbtcore/interfaces/peersource.cpp
    libbtcore/interfaces/torrentinterface.cpp
    libbtcore/interfaces/torrentfileinterface.cpp
    libbtcore/interfaces/trackerslist.cpp
    libbtcore/interfaces/trackerinterface.cpp
    libbtcore/interfaces/logmonitorinterface.cpp
    libbtcore/interfaces/blocklistinterface.cpp
    libbtcore/interfaces/exitoperation.cpp
    libbtcore/interfaces/chunkdownloadinterface.cpp
    libbtcore/interfaces/monitorinterface.cpp
    libbtcore/interfaces/queuemanagerinterface.cpp
    libbtcore/interfaces/chunkselectorinterface.cpp
    libbtcore/interfaces/cachefactory.cpp
    libbtcore/interfaces/webseedinterface.cpp

    libbtcore/torrent/globals.cpp
    libbtcore/torrent/server.cpp
    libbtcore/torrent/torrent.cpp
    libbtcore/torrent/torrentfile.cpp
    libbtcore/torrent/peersourcemanager.cpp
    libbtcore/torrent/uploader.cpp
    libbtcore/torrent/timeestimator.cpp
    libbtcore/torrent/statsfile.cpp
    libbtcore/torrent/choker.cpp
    libbtcore/torrent/advancedchokealgorithm.cpp
    libbtcore/torrent/torrentcontrol.cpp
    libbtcore/torrent/torrentcreator.cpp
    libbtcore/torrent/torrentstats.cpp
    libbtcore/torrent/job.cpp
    libbtcore/torrent/jobqueue.cpp

    libbtcore/dht/announcetask.cpp  
    libbtcore/dht/dht.cpp                
    libbtcore/dht/kclosestnodessearch.cpp  
    libbtcore/dht/nodelookup.cpp  
    libbtcore/dht/rpcmsg.cpp     
    libbtcore/dht/taskmanager.cpp
    libbtcore/dht/database.cpp      
    libbtcore/dht/dhttrackerbackend.cpp  
    libbtcore/dht/key.cpp                  
    libbtcore/dht/pack.cpp        
    libbtcore/dht/rpcserver.cpp
    libbtcore/dht/dhtbase.cpp       
    libbtcore/dht/kbucket.cpp            
    libbtcore/dht/node.cpp                 
    libbtcore/dht/rpccall.cpp     
    libbtcore/dht/task.cpp

    libbtcore/diskio/cache.cpp      
    libbtcore/diskio/chunk.cpp         
    libbtcore/diskio/dndfile.cpp         
    libbtcore/diskio/singlefilecache.cpp
    libbtcore/diskio/multifilecache.cpp
    libbtcore/diskio/preallocationthread.cpp
    libbtcore/diskio/movedatafilesjob.cpp
    libbtcore/diskio/deletedatafilesjob.cpp
    libbtcore/diskio/piecedata.cpp
    libbtcore/diskio/cachefile.cpp  
    libbtcore/diskio/chunkmanager.cpp  
    libbtcore/diskio/preallocationjob.cpp

    libbtcore/tracker/httptracker.cpp  
    libbtcore/tracker/tracker.cpp  
    libbtcore/tracker/udptracker.cpp  
    libbtcore/tracker/udptrackersocket.cpp
    libbtcore/tracker/httpannouncejob.cpp
    libbtcore/tracker/trackermanager.cpp
    
    libbtcore/datachecker/datachecker.cpp          
    libbtcore/datachecker/datacheckerthread.cpp  
    libbtcore/datachecker/singledatachecker.cpp
    libbtcore/datachecker/datacheckerlistener.cpp  
    libbtcore/datachecker/multidatachecker.cpp
    libbtcore/datachecker/datacheckerjob.cpp
    
    libbtcore/migrate/cachemigrate.cpp  
    libbtcore/migrate/ccmigrate.cpp  
    libbtcore/migrate/migrate.cpp
    
    libbtcore/btversion.cpp
)

if(WIN32)
# Compile stuff for windows
set(libbtcore_SRC ${libbtcore_SRC} libbtcore/util/win32.cpp)
endif(WIN32)

else (ENABLE_EMBEDDED_TORRENT_SUPPORT AND GMP_FOUND AND QCA2_FOUND)
    find_package(BTCore REQUIRED)
    macro_log_feature(BTCORE_FOUND "BTCore" "KTorrent core library" "http://ktorrent.org/" FALSE "" "Needed to build Torrent support in KGet. Please install it or pass -DENABLE_EMBEDDED_TORRENT_SUPPORT=true to build embedded torrent support.")

    include_directories(
        ../../
       ${BTCORE_INCLUDE_DIR}
    )
endif (ENABLE_EMBEDDED_TORRENT_SUPPORT AND GMP_FOUND AND QCA2_FOUND)

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

if (GMP_FOUND AND QCA2_FOUND AND ENABLE_EMBEDDED_TORRENT_SUPPORT)
    kde4_add_kcfg_files(kget_bittorrentfactory_PART_SRCS bittorrentsettings.kcfgc)
    kde4_add_plugin(kget_bittorrentfactory ${kget_bittorrentfactory_PART_SRCS} ${libbtcore_SRC})
    if (HAVE_NEPOMUK)
        target_link_libraries(kget_bittorrentfactory ${SOPRANO_LIBRARIES} ${NEPOMUK_LIBRARIES})
    endif (HAVE_NEPOMUK)
    target_link_libraries(kget_bittorrentfactory ${KDE4_KIO_LIBS} ${GMP_LIBRARIES} ${QCA2_LIBRARIES} kgetcore)
    install(TARGETS kget_bittorrentfactory DESTINATION  ${PLUGIN_INSTALL_DIR})
    install(FILES kget_bittorrentfactory.desktop DESTINATION ${SERVICES_INSTALL_DIR})
else (GMP_FOUND AND QCA2_FOUND AND ENABLE_EMBEDDED_TORRENT_SUPPORT)
    kde4_add_kcfg_files(kget_bittorrentfactory_PART_SRCS bittorrentsettings.kcfgc)
    kde4_add_plugin(kget_bittorrentfactory ${kget_bittorrentfactory_PART_SRCS})
    if (HAVE_NEPOMUK)
        target_link_libraries(kget_bittorrentfactory ${SOPRANO_LIBRARIES} ${NEPOMUK_LIBRARIES})
    endif (HAVE_NEPOMUK)
    target_link_libraries(kget_bittorrentfactory ${KDE4_KIO_LIBS} ${BTCORE_LIBRARIES} kgetcore)
    install(TARGETS kget_bittorrentfactory DESTINATION  ${PLUGIN_INSTALL_DIR})
    install(FILES kget_bittorrentfactory.desktop DESTINATION ${SERVICES_INSTALL_DIR})
endif (GMP_FOUND AND QCA2_FOUND AND ENABLE_EMBEDDED_TORRENT_SUPPORT)

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
