# - Try to find the libmms library
# Once done this will define
#
#  LIBMMS_FOUND - system has libmms
#  LIBMMS_INCLUDE_DIR - the libmms include directory
#  LIBMMS_LIBRARIES - Link these to use libmms

# Copyright (c) 2007 Joris Guisson <joris.guisson@gmail.com>
# Copyright (c) 2007 Charles Connell <charles@connells.org> (This was based upon FindKopete.cmake)
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(LIBMMS_INCLUDE_DIR AND LIBMMS_LIBRARIES)

  # read from cache
  set(LIBMMS_FOUND TRUE)

else(LIBMMS_INCLUDE_DIR AND LIBMMS_LIBRARIES)

  FIND_PATH(LIBMMS_INCLUDE_DIR 
    NAMES
    mmsx.h
    PATHS
    ${INCLUDE_INSTALL_DIR}
    PATH_SUFFIXES
    libmms
    )
  
  FIND_LIBRARY(LIBMMS_LIBRARIES 
    NAMES
    mms
    PATHS
    ${LIB_INSTALL_DIR}
    )
  if(LIBMMS_INCLUDE_DIR AND LIBMMS_LIBRARIES)
    set(LIBMMS_FOUND TRUE)
  endif(LIBMMS_INCLUDE_DIR AND LIBMMS_LIBRARIES)

  if(LIBMMS_FOUND)
    if(NOT LIBKTORRENT_FIND_QUIETLY)
      message(STATUS "Found libmms: ${LIBMMS_LIBRARIES} ")
    endif(NOT LIBKTORRENT_FIND_QUIETLY)
  else(LIBMMS_FOUND)
    if(LIBKTORRENT_FIND_REQUIRED)
      if(NOT LIBMMS_INCLUDE_DIR)
    message(FATAL_ERROR "Could not find libmms includes.")
      endif(NOT LIBMMS_INCLUDE_DIR)
      if(NOT LIBMMS_LIBRARIES)
    message(FATAL_ERROR "Could not find libmms library.")
      endif(NOT LIBMMS_LIBRARIES)
    else(LIBKTORRENT_FIND_REQUIRED)
      if(NOT LIBMMS_INCLUDE_DIR)
        message(STATUS "Could not find libmms includes.")
      endif(NOT LIBMMS_INCLUDE_DIR)
      if(NOT LIBMMS_LIBRARIES)
        message(STATUS "Could not find libmms library.")
      endif(NOT LIBMMS_LIBRARIES)
    endif(LIBKTORRENT_FIND_REQUIRED)
  endif(LIBMMS_FOUND)

endif(LIBMMS_INCLUDE_DIR AND LIBMMS_LIBRARIES)
