install(DIRECTORY widget-qml/
        DESTINATION ${DATA_INSTALL_DIR}/plasma/plasmoids/org.kde.kget.widget)
 
install(FILES widget-qml/metadata.desktop
        DESTINATION ${SERVICES_INSTALL_DIR} RENAME kget-widget.desktop)