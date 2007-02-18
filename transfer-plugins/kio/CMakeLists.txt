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

kde4_automoc(${kget_kiofactory_PART_SRCS})

kde4_add_plugin(kget_kiofactory WITH_PREFIX ${kget_kiofactory_PART_SRCS})

target_link_libraries(kget_kiofactory ${KDE4_KDECORE_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KDEUI_LIBS} kgetcore)

install(TARGETS kget_kiofactory DESTINATION ${LIB_INSTALL_DIR}/kde4)

########### install files ###############

install(FILES kget_kiofactory.desktop DESTINATION ${SERVICES_INSTALL_DIR})
