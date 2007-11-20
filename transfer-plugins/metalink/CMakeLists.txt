include_directories(
   ../../
)

set(kget_metalinkfactory_PART_SRCS
  metalinkfactory.cpp
  metalink.cpp
  metalinker.cpp
)

kde4_add_plugin(kget_metalinkfactory ${kget_metalinkfactory_PART_SRCS})

target_link_libraries(kget_metalinkfactory ${KDE4_KIO_LIBS} kgetcore)

install(TARGETS kget_metalinkfactory DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kget_metalinkfactory.desktop DESTINATION ${SERVICES_INSTALL_DIR})
