#Set supported mime type
SET(SUPPORTED_KGET_MIMETYPES "${SUPPORTED_KGET_MIMETYPES}application/metalink+xml;" PARENT_SCOPE)

include_directories(
   ../../
)

set(kget_metalinkfactory_PART_SRCS
  metalinkfactory.cpp
  abstractmetalink.cpp
  metalinkxml.cpp
  metalinkhttp.cpp
  fileselectiondlg.cpp
  ../../ui/metalinkcreator/metalinker.cpp
)

qt5_wrap_ui(kget_metalinkfactory_PART_SRCS
    fileselection.ui
)

kconfig_add_kcfg_files(kget_metalinkfactory_PART_SRCS metalinksettings.kcfgc)

add_library(kget_metalinkfactory MODULE ${kget_metalinkfactory_PART_SRCS})

target_link_libraries(kget_metalinkfactory ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} kgetcore)
if (HAVE_NEPOMUK)
    target_link_libraries(kget_metalinkfactory ${SOPRANO_LIBRARIES} ${NEPOMUK_CORE_LIBRARY})
endif (HAVE_NEPOMUK)

install(TARGETS kget_metalinkfactory DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kget_metalinkfactory.desktop DESTINATION ${SERVICES_INSTALL_DIR})


###Build KCM-Module
set(kcm_kget_metalinkfactory_PART_SRCS
dlgmetalink.cpp
)

qt5_wrap_ui(kcm_kget_metalinkfactory_PART_SRCS
dlgmetalink.ui
)

kconfig_add_kcfg_files(kcm_kget_metalinkfactory_PART_SRCS metalinksettings.kcfgc)
add_library(kcm_kget_metalinkfactory MODULE ${kcm_kget_metalinkfactory_PART_SRCS})
target_link_libraries(kcm_kget_metalinkfactory ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS})
install(TARGETS kcm_kget_metalinkfactory DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES kget_metalinkfactory_config.desktop DESTINATION ${SERVICES_INSTALL_DIR})
