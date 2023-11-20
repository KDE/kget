add_library(kget_kio MODULE)

target_sources(kget_kio PRIVATE
  transferKio.cpp
  transferKioFactory.cpp
  kget_kiofactory.json
  ../../kget_debug.cpp
)

target_link_libraries(kget_kio kgetcore)

install(TARGETS kget_kio DESTINATION ${KGET_PLUGIN_INSTALL_DIR})
