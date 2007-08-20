project(kget)

add_subdirectory(conf)
add_subdirectory(core)
add_subdirectory(ui)
add_subdirectory(transfer-plugins)
add_subdirectory(extensions)
add_subdirectory(sounds)
add_subdirectory(desktop)

include_directories(
   ${CMAKE_SOURCE_DIR}/kget/core/
   ${CMAKE_CURRENT_BINARY_DIR}
)


# kgetcore

set(kgetcore_SRCS
   core/job.cpp
   core/jobqueue.cpp
   core/kget.cpp
   core/scheduler.cpp
   core/transfertreemodel.cpp
   core/transfertreeselectionmodel.cpp
   core/transfer.cpp
   core/transfergroup.cpp
   core/transfergrouphandler.cpp
   core/transferhandler.cpp
   core/plugin/plugin.cpp
   core/plugin/transferfactory.cpp
   core/transfercontainer.cpp
)

kde4_add_kcfg_files(kgetcore_SRCS conf/settings.kcfgc)


kde4_add_library(kgetcore SHARED ${kgetcore_SRCS})

target_link_libraries(kgetcore ${KDE4_KIO_LIBS})
set_target_properties(kgetcore PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kgetcore DESTINATION ${LIB_INSTALL_DIR})


# kget

set(kget_SRCS
   conf/dlgdirectories.cpp
   conf/preferencesdialog.cpp
   dbus/dbusmodelobserver.cpp
   dbus/dbustransfergroupobserver.cpp
   ui/droptarget.cpp
   ui/transfersview.cpp
   ui/transfersviewdelegate.cpp
   ui/groupseditdialog.cpp
   ui/splash.cpp
   ui/transferdetails.cpp
   ui/tray.cpp
   ui/viewscontainer.cpp
   mainwindow.cpp
   main.cpp
)

qt4_add_dbus_adaptor(kget_SRCS org.kde.kget.xml mainwindow.h MainWindow)

kde4_add_ui_files(kget_SRCS
   conf/dlgadvanced.ui
   conf/dlgappearance.ui
   conf/dlgdirectories.ui
   conf/dlgnetwork.ui
   ui/transferdetailsfrm.ui
)

kde4_add_executable(kget ${kget_SRCS})

target_link_libraries(kget ${KDE4_KDEUI_LIBRARY} ${KDE4_KIO_LIBRARY} ${KDE4_KNOTIFYCONFIG_LIBRARY} kgetcore)

install(TARGETS kget DESTINATION ${BIN_INSTALL_DIR})
