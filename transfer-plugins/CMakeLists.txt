add_subdirectory(kio)
#doesn't work currently, needs to be fixed
#add_subdirectory(multisegmentkio)
#requires multisegmentkio plugin
#add_subdirectory(metalink)
if (KF5Torrent_FOUND)
  add_subdirectory(bittorrent)
endif (KF5Torrent_FOUND)
add_subdirectory(mirrorsearch)
#add_subdirectory(contentfetch)
if (LIBMMS_FOUND)
  add_subdirectory(mmsthreads)
endif (LIBMMS_FOUND)
add_subdirectory(checksumsearch)

SET(SUPPORTED_KGET_MIMETYPES "${SUPPORTED_KGET_MIMETYPES}" PARENT_SCOPE)
