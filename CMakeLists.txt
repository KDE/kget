project(kget)

set(kget_adaptor_SRCS)

macro_bool_to_01(NEPOMUK_FOUND HAVE_NEPOMUK)
if(HAVE_NEPOMUK)
    include_directories(
        ${SOPRANO_INCLUDE_DIR}
        ${NEPOMUK_INCLUDE_DIR}
    )
    add_definitions(-DHAVE_NEPOMUK)
    include(NepomukAddOntologyClasses)
endif(HAVE_NEPOMUK)

if(QCA2_FOUND)
    add_definitions(-DHAVE_QCA2)
endif(QCA2_FOUND)

qt4_add_dbus_adaptor(kget_adaptor_SRCS org.kde.kget.xml mainwindow.h MainWindow)

add_subdirectory(conf)
add_subdirectory(core)
add_subdirectory(ui)
add_subdirectory(transfer-plugins)
add_subdirectory(extensions)
add_subdirectory(sounds)
add_subdirectory(desktop)
add_subdirectory(plasma)
add_subdirectory(tests)


# find kworkspace library to allow the shutdown after downloads completed option
find_library(KDE4_KWORKSPACE_LIBRARY NAMES kworkspace PATHS ${KDE4_LIB_INSTALL_DIR} NO_DEFAULT_PATH )
if(KDE4_KWORKSPACE_LIBRARY)
    set(KDE4_KWORKSPACE_LIBS ${kworkspace_LIB_DEPENDS} ${KDE4_KWORKSPACE_LIBRARY})
    add_definitions(-DHAVE_KWORKSPACE)
    set(KWORKSPACE_FOUND true)
endif(KDE4_KWORKSPACE_LIBRARY)

# find libkonq to allow actions and open with options in the context menu of a transfer
find_library(KDE4_KONQUEROR_LIBRARY NAMES konq PATHS ${KDE4_LIB_INSTALL_DIR} NO_DEFAULT_PATH )
if(KDE4_KONQUEROR_LIBRARY)
    set(KDE4_KONQUEROR_LIBS ${konq_LIB_DEPENDS} ${KDE4_KONQUEROR_LIBRARY})
    add_definitions(-DHAVE_KONQUEROR)
    set(KONQUEROR_FOUND true)
endif(KDE4_KONQUEROR_LIBRARY)


include_directories(
   ${CMAKE_SOURCE_DIR}/kget/core/
   ${CMAKE_CURRENT_BINARY_DIR}
)

if(SQLITE_FOUND)
    add_definitions(-DHAVE_SQLITE)
endif(SQLITE_FOUND)

if (LIBKNOTIFICATIONITEM-1_FOUND)
    add_definitions(-DHAVE_KNOTIFICATIONITEM)
    include_directories(${LIBKNOTIFICATIONITEM-1_INCLUDE_DIR})
endif (LIBKNOTIFICATIONITEM-1_FOUND)

remove_definitions(-DQT_NO_HTTP)

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
   core/handler.cpp
   core/transfergroupscheduler.cpp
   core/plugin/plugin.cpp
   core/plugin/transferfactory.cpp
   core/transferdatasource.cpp
   core/datasourcefactory.cpp
   core/kgetkjobadapter.cpp
   core/kuiserverjobs.cpp
   core/kgetglobaljob.cpp
   core/bitset.cpp
   core/download.cpp
   core/transferhistorystore.cpp
   core/linkimporter.cpp
)

if(HAVE_NEPOMUK)
    set(kgetcore_SRCS
       ${kgetcore_SRCS}
       core/nepomukhandler.cpp
    )
endif(HAVE_NEPOMUK)

kde4_add_kcfg_files(kgetcore_SRCS conf/settings.kcfgc)

if (HAVE_NEPOMUK)
    NEPOMUK_ADD_ONTOLOGY_CLASSES(
        kgetcore_SRCS
        ONTOLOGIES
        ${CMAKE_CURRENT_SOURCE_DIR}/kget_history.trig
    )
endif (HAVE_NEPOMUK)


kde4_add_library(kgetcore SHARED ${kgetcore_SRCS})

target_link_libraries(kgetcore ${KDE4_KIO_LIBS})

if (KWORKSPACE_FOUND)
    target_link_libraries(kgetcore ${KDE4_KWORKSPACE_LIBS})
endif (KWORKSPACE_FOUND)

if (HAVE_NEPOMUK)
  target_link_libraries(kgetcore ${SOPRANO_LIBRARIES} ${KDE4_NEPOMUK_LIBS})
endif (HAVE_NEPOMUK)

if (SQLITE_FOUND)
  target_link_libraries(kgetcore ${QT_QTSQL_LIBRARY})
endif (SQLITE_FOUND)

if (LIBKNOTIFICATIONITEM-1_FOUND)
  target_link_libraries(kgetcore ${LIBKNOTIFICATIONITEM-1_LIBRARIES})
endif (LIBKNOTIFICATIONITEM-1_FOUND)

set_target_properties(kgetcore PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kgetcore ${INSTALL_TARGETS_DEFAULT_ARGS})


# kget

set(kget_SRCS ${kget_adaptor_SRCS}
   conf/dlgwebinterface.cpp
   conf/preferencesdialog.cpp
   conf/transfersgroupwidget.cpp
   conf/pluginselector.cpp
   dbus/dbusmodelobserver.cpp
   dbus/dbustransfergroupobserver.cpp
   ui/droptarget.cpp
   ui/transfersview.cpp
   ui/transfersviewdelegate.cpp
   ui/splash.cpp
   ui/transferdetails.cpp
   ui/viewscontainer.cpp
   ui/newtransferdialog.cpp
   ui/groupsettingsdialog.cpp
   ui/transfersettingsdialog.cpp
   ui/contextmenu.cpp
   ui/history/rangetreewidget.cpp
   ui/history/transferhistory.cpp
   ui/history/transferhistoryitemdelegate.cpp
   ui/history/transferhistorycategorizeddelegate.cpp
   ui/history/transferhistorycategorizedview.cpp
   ui/linkview/kget_linkview.cpp
   ui/linkview/kget_sortfilterproxymodel.cpp
   tests/testkget.cpp
   mainwindow.cpp
   main.cpp

   extensions/webinterface/httpserver.cpp
)

if(LIBKNOTIFICATIONITEM-1_FOUND)
    set(kget_SRCS
        ${kget_SRCS}
        ui/tray_newproto.cpp
    )
else(LIBKNOTIFICATIONITEM-1_FOUND)
    set(kget_SRCS
        ${kget_SRCS}
        ui/tray.cpp
    )
endif(LIBKNOTIFICATIONITEM-1_FOUND)

if(HAVE_NEPOMUK)
   set(kget_SRCS
       ${kget_SRCS}
       ui/nepomukwidget.cpp
   )
endif(HAVE_NEPOMUK)

kde4_add_ui_files(kget_SRCS
   conf/dlgadvanced.ui
   conf/dlgappearance.ui
   conf/dlgwebinterface.ui
   conf/dlgnetwork.ui
   ui/transferdetailsfrm.ui
   ui/newtransferwidget.ui
   ui/history/transferhistory.ui
   ui/groupsettingsdialog.ui
   ui/transfersettingsdialog.ui
   ui/linkview/importlinkdialog.ui
)

if(HAVE_NEPOMUK)
    kde4_add_ui_files(kget_SRCS
       ui/grouptagssettingsdialog.ui
       ui/nepomukwidget.ui
   )
endif(HAVE_NEPOMUK)

kde4_add_app_icon(kget_SRCS "ui/icons/hi*-apps-kget.png")

kde4_add_executable(kget ${kget_SRCS})

target_link_libraries(kget ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBRARY} ${KDE4_KIO_LIBRARY} ${KDE4_KNOTIFYCONFIG_LIBRARY} ${KDE4_KUTILS_LIBRARY} kgetcore)

if (HAVE_NEPOMUK)
  target_link_libraries(kget ${KDE4_NEPOMUK_LIBS})
endif (HAVE_NEPOMUK)

if (KONQUEROR_FOUND)
  target_link_libraries(kget ${KDE4_KONQUEROR_LIBS})
endif (KONQUEROR_FOUND)

if (LIBKNOTIFICATIONITEM-1_FOUND)
  target_link_libraries(kget ${LIBKNOTIFICATIONITEM-1_LIBRARIES})
endif (LIBKNOTIFICATIONITEM-1_FOUND)

install(TARGETS kget ${INSTALL_TARGETS_DEFAULT_ARGS})

