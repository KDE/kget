include_directories(
   ../../
    ${CMAKE_CURRENT_BINARY_DIR}/../../
)

set(kget_kiofactory_PART_SRCS
  transferKio.cpp
  transferKioFactory.cpp
)

add_library(kget_kiofactory MODULE ${kget_kiofactory_PART_SRCS})

target_link_libraries(kget_kiofactory ${KDE4_KIO_LIBS} kgetcore)

install(TARGETS kget_kiofactory DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kget_kiofactory.desktop DESTINATION ${SERVICES_INSTALL_DIR})
