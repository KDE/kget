# comment only one of the two icon folders. this is just a dirty hack and will be improved soon.
add_subdirectory(oxygen)
# add_subdirectory(crystal)

install(FILES kget.desktop DESTINATION ${XDG_APPS_DIR})
install(FILES x-kgetlist.desktop DESTINATION ${MIME_INSTALL_DIR}/application)
install(FILES kget_download.desktop DESTINATION ${DATA_INSTALL_DIR}/konqueror/servicemenus)
