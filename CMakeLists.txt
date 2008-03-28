project(kget)

set(kget_adaptor_SRCS)

macro_bool_to_01(Nepomuk_FOUND HAVE_NEPOMUK)

qt4_add_dbus_adaptor(kget_adaptor_SRCS org.kde.kget.xml mainwindow.h MainWindow)

add_subdirectory(conf)
add_subdirectory(core)
add_subdirectory(ui)
add_subdirectory(transfer-plugins)
add_subdirectory(extensions)
add_subdirectory(sounds)
add_subdirectory(desktop)
if(PLASMA_FOUND)
    add_subdirectory(plasma)
endif(PLASMA_FOUND)

include_directories(
   ${CMAKE_SOURCE_DIR}/kget/core/
   ${CMAKE_CURRENT_BINARY_DIR}
)
if(Nepomuk_FOUND)
    include_directories(
        ${SOPRANO_INCLUDE_DIR}
        ${NEPOMUK_INCLUDE_DIR}
    )
    add_definitions(-DHAVE_NEPOMUK)
endif(Nepomuk_FOUND)

if(SQLITE_FOUND)
    add_definitions(-DHAVE_SQLITE)
endif(SQLITE_FOUND)

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
   core/transfergroupscheduler.cpp
   core/plugin/plugin.cpp
   core/plugin/transferfactory.cpp
   core/transferdatasource.cpp
   core/datasourcefactory.cpp
   core/kuiserverjobs.cpp
   core/kgetglobaljob.cpp
   core/bitset.cpp
   core/download.cpp
   core/transferhistorystore.cpp
)

kde4_add_kcfg_files(kgetcore_SRCS conf/settings.kcfgc)


kde4_add_library(kgetcore SHARED ${kgetcore_SRCS})

target_link_libraries(kgetcore ${KDE4_KIO_LIBS})

if (Nepomuk_FOUND)
  target_link_libraries(kgetcore ${SOPRANO_LIBRARIES} ${NEPOMUK_LIBRARIES})
endif (Nepomuk_FOUND)

if (SQLITE_FOUND)
  target_link_libraries(kgetcore ${QT_QTSQL_LIBRARY})
endif (SQLITE_FOUND)

set_target_properties(kgetcore PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kgetcore ${INSTALL_TARGETS_DEFAULT_ARGS})


# kget

set(kget_SRCS ${kget_adaptor_SRCS}
   conf/dlgdirectories.cpp
   conf/dlgwebinterface.cpp
   conf/preferencesdialog.cpp
   conf/transfersgroupwidget.cpp
   conf/selectdirectoryitemdelegate.cpp
   dbus/dbusmodelobserver.cpp
   dbus/dbustransfergroupobserver.cpp
   ui/droptarget.cpp
   ui/transfersview.cpp
   ui/transfersviewdelegate.cpp
   ui/splash.cpp
   ui/transferdetails.cpp
   ui/tray.cpp
   ui/viewscontainer.cpp
   ui/newtransferdialog.cpp
   ui/groupsettingsdialog.cpp
   ui/transfersettingsdialog.cpp
   ui/history/rangetreewidget.cpp
   ui/history/transferhistory.cpp
   ui/history/transferhistoryitemdelegate.cpp
   ui/history/transferhistorycategorizeddelegate.cpp
   ui/history/transferhistorycategorizedview.cpp
   mainwindow.cpp
   main.cpp

   extensions/webinterface/httpserver.cpp
)

kde4_add_ui_files(kget_SRCS
   conf/dlgadvanced.ui
   conf/dlgappearance.ui
   conf/dlgdirectories.ui
   conf/dlgwebinterface.ui
   conf/dlgnetwork.ui
   ui/transferdetailsfrm.ui
   ui/newtransferwidget.ui
   ui/history/transferhistory.ui
   ui/groupsettingsdialog.ui
   ui/transfersettingsdialog.ui
)

kde4_add_executable(kget ${kget_SRCS})

target_link_libraries(kget ${KDE4_KDEUI_LIBRARY} ${KDE4_KIO_LIBRARY} ${KDE4_KNOTIFYCONFIG_LIBRARY} kgetcore)

install(TARGETS kget DESTINATION ${BIN_INSTALL_DIR})
