set(www_data
    index.htm
    mootools.js
)

install(FILES ${www_data} DESTINATION ${DATA_INSTALL_DIR}/kget/www)
