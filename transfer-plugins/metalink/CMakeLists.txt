#Set supported mime type
SET(SUPPORTED_KGET_MIMETYPES "${SUPPORTED_KGET_MIMETYPES}application/metalink+xml;" PARENT_SCOPE)

include_directories(
   ../../
)

set(kget_metalinkfactory_PART_SRCS
  metalinkfactory.cpp
  metalink.cpp
  fileselectiondlg.cpp
  ../../ui/metalinkcreator/metalinker.cpp
)

kde4_add_ui_files(kget_metalinkfactory_PART_SRCS
    fileselection.ui
)

kde4_add_kcfg_files(kget_metalinkfactory_PART_SRCS metalinksettings.kcfgc)

kde4_add_plugin(kget_metalinkfactory ${kget_metalinkfactory_PART_SRCS})

target_link_libraries(kget_metalinkfactory ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} kgetcore)
if (HAVE_NEPOMUK)
    target_link_libraries(kget_metalinkfactory ${SOPRANO_LIBRARIES} ${NEPOMUK_LIBRARIES})
endif (HAVE_NEPOMUK)

install(TARGETS kget_metalinkfactory DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kget_metalinkfactory.desktop DESTINATION ${SERVICES_INSTALL_DIR})


###Build KCM-Module
set(kcm_kget_metalinkfactory_PART_SRCS
dlgmetalink.cpp
)

kde4_add_ui_files(kcm_kget_metalinkfactory_PART_SRCS
dlgmetalink.ui
)

kde4_add_kcfg_files(kcm_kget_metalinkfactory_PART_SRCS metalinksettings.kcfgc)
kde4_add_plugin(kcm_kget_metalinkfactory ${kcm_kget_metalinkfactory_PART_SRCS})
target_link_libraries(kcm_kget_metalinkfactory ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS})
install(TARGETS kcm_kget_metalinkfactory DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES kget_metalinkfactory_config.desktop DESTINATION ${SERVICES_INSTALL_DIR})