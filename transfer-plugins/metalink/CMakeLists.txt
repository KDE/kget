include_directories(
   ${CMAKE_SOURCE_DIR}/kget/
   ${KDE4_INCLUDES}
   ${QT_INCLUDES}
)

########### next target ###############

set(kget_metalinkfactory_PART_SRCS
  ../multisegmentkio/segmentfactory.cpp
  ../multisegmentkio/multisegkio.cpp
  metalinkfactory.cpp
  metalink.cpp
  metalinker.cpp
)

kde4_automoc(${kget_metalinkfactory_PART_SRCS})

kde4_add_kcfg_files(kget_metalinkfactory_PART_SRCS ../multisegmentkio/multisegkiosettings.kcfgc)

kde4_add_plugin(kget_metalinkfactory WITH_PREFIX ${kget_metalinkfactory_PART_SRCS})

target_link_libraries(kget_metalinkfactory ${KDE4_KIO_LIBS} ${KDE4_KDEUI_LIBS} kgetcore)

install(TARGETS kget_metalinkfactory DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install(FILES kget_metalinkfactory.desktop DESTINATION ${SERVICES_INSTALL_DIR})
