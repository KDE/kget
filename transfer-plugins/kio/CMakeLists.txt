include_directories(
   ${CMAKE_SOURCE_DIR}/kget/
   ${KDE4_INCLUDES}
   ${QT_INCLUDES}
)

########### next target ###############

set(kget_kiofactory_PART_SRCS
  transferKio.cpp
  transferKioFactory.cpp
)


kde4_add_plugin(kget_kiofactory WITH_PREFIX ${kget_kiofactory_PART_SRCS})

target_link_libraries(kget_kiofactory ${KDE4_KIO_LIBS}  kgetcore)

install(TARGETS kget_kiofactory DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install(FILES kget_kiofactory.desktop DESTINATION ${SERVICES_INSTALL_DIR})
