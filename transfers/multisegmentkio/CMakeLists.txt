
include_directories(
   ${CMAKE_SOURCE_DIR}/kget/
   ${KDE4_INCLUDE_DIR}
   ${QT_INCLUDES}
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_BINARY_DIR}
)

########### next target ###############

set(kget_MultiSegKiofactory_PART_SRCS
  MultiSegKio.cpp
  transferMultiSegKio.cpp
  transferMultiSegKioFactory.cpp
)

kde4_automoc(${kget_MultiSegKiofactory_PART_SRCS})

kde4_add_plugin(kget_MultiSegKiofactory WITH_PREFIX ${kget_MultiSegKiofactory_PART_SRCS})

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kget_MultiSegKiofactory)

target_link_libraries(kget_MultiSegKiofactory ${KDE4_KDECORE_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KDEUI_LIBS} kgetcore)

install_targets(TARGETS kget_MultiSegKiofactory DESTINATION ${LIB_INSTALL_DIR}/kde4)

########### install files ###############

install_files(FILES  kget_MultiSegKiofactory.desktop DESTINATION  ${SERVICES_INSTALL_DIR})

add_subdirectory(test)

