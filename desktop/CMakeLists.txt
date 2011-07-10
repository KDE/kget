# associate KGet with the correct mimetypes
configure_file(kget.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kget.desktop)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/kget.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES kget_download.desktop DESTINATION
${SERVICES_INSTALL_DIR}/ServiceMenus)

# For DBus activation:
configure_file(org.kde.kget.service.in
    ${CMAKE_CURRENT_BINARY_DIR}/org.kde.kget.service)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.kget.service
    DESTINATION ${DBUS_SERVICES_INSTALL_DIR})
