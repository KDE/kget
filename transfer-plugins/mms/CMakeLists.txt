include_directories(
   ../../
   /usr/include/libmms/
   /usr/include/
)

set(kget_mmsfactory_PART_SRCS
  mmsTransfer.cpp
  mmsTransferFactory.cpp
)

kde4_add_plugin(kget_mmsfactory ${kget_mmsfactory_PART_SRCS})

target_link_libraries(kget_mmsfactory ${KDE4_KIO_LIBS} kgetcore -lmms)

install(TARGETS kget_mmsfactory DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kget_mmsfactory.desktop DESTINATION ${SERVICES_INSTALL_DIR})
