add_library(kget_kio MODULE)

target_sources(kget_kio PRIVATE
  transferKio.cpp
  transferKioFactory.cpp
  ../../kget_debug.cpp
)

kcoreaddons_desktop_to_json(kget_kio kget_kiofactory.desktop)
target_link_libraries(kget_kio kgetcore)

install(TARGETS kget_kio DESTINATION ${KGET_PLUGIN_INSTALL_DIR})
