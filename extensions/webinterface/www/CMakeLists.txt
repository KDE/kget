set(www_data
    index.htm
    mootools-core-yc.js
    mootools-more.js
    favicon.ico
    header-background.png
    header-separator.png
    hi64-apps-kget.png
    new-download-background.png
    png_hack/blank.gif
    png_hack/iepngfix.htc
    preferences-system.png
    title-border-bottom-downloads.png
    title-border-bottom-settings.png
    view-refresh.png
)

install(FILES ${www_data} DESTINATION ${DATA_INSTALL_DIR}/kget/www)
