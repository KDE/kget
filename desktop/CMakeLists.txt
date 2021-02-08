# associate KGet with the correct mimetypes
configure_file(org.kde.kget.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/org.kde.kget.desktop)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/org.kde.kget.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES kget_download.desktop DESTINATION
${KDE_INSTALL_KSERVICES5DIR}/ServiceMenus)

# For DBus activation:
configure_file(org.kde.kget.service.in
    ${CMAKE_CURRENT_BINARY_DIR}/org.kde.kget.service)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.kget.service
    DESTINATION ${KDE_INSTALL_DBUSSERVICEDIR})
