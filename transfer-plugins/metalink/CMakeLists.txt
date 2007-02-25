include_directories(
   ${CMAKE_SOURCE_DIR}/kget/
   ${KDE4_INCLUDES}
   ${QT_INCLUDES}
)

########### next target ###############

set(kget_metalinkfactory_PART_SRCS
  ../multisegmentkio/segmentfactory.cpp
  ../multisegmentkio/MultiSegKio.cpp
  metalinkFactory.cpp
  metalink.cpp
  metalinker.cpp
)

kde4_automoc(${kget_metalinkfactory_PART_SRCS})

kde4_add_plugin(kget_metalinkfactory WITH_PREFIX ${kget_metalinkfactory_PART_SRCS})

target_link_libraries(kget_metalinkfactory ${KDE4_KDECORE_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KDEUI_LIBS} kgetcore)

install(TARGETS kget_metalinkfactory DESTINATION ${LIB_INSTALL_DIR}/kde4)

########### install files ###############

install(FILES kget_metalinkfactory.desktop DESTINATION ${SERVICES_INSTALL_DIR})
