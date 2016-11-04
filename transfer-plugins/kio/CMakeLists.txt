include_directories(
   ../../
    ${CMAKE_CURRENT_BINARY_DIR}/../../
)

set(kget_kiofactory_PART_SRCS
  transferKio.cpp
  transferKioFactory.cpp
  ../../kget_debug.cpp
)

add_library(kget_kio MODULE ${kget_kiofactory_PART_SRCS})
kcoreaddons_desktop_to_json(kget_kio kget_kiofactory.desktop)
target_link_libraries(kget_kio kgetcore)

install(TARGETS kget_kio DESTINATION ${KGET_PLUGIN_INSTALL_DIR})
