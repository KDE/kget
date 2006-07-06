add_subdirectory(conf)
add_subdirectory(core)
add_subdirectory(ui)
add_subdirectory(transfers)
add_subdirectory(plugins)
add_subdirectory(sounds)

include_directories(
   ${CMAKE_SOURCE_DIR}/kget/core/
   ${KDE4_INCLUDE_DIR}
   ${QT_INCLUDES}
   ${QDBUS_INCLUDE_DIRS}
)

########### next target ###############

set(kgetconf_SRCS
   ${CMAKE_SOURCE_DIR}/kget/conf/preferencesdialog.cpp
)

kde4_add_ui_files(kgetconf_SRCS
   ${CMAKE_SOURCE_DIR}/kget/conf/dlgadvanced.ui
   ${CMAKE_SOURCE_DIR}/kget/conf/dlgappearance.ui
   ${CMAKE_SOURCE_DIR}/kget/conf/dlgdirectories.ui
   ${CMAKE_SOURCE_DIR}/kget/conf/dlgnetwork.ui
)

set(kgetcore_SRCS
   ${CMAKE_SOURCE_DIR}/kget/core/connection.cpp
   ${CMAKE_SOURCE_DIR}/kget/core/job.cpp
   ${CMAKE_SOURCE_DIR}/kget/core/jobqueue.cpp
   ${CMAKE_SOURCE_DIR}/kget/core/model.cpp
   ${CMAKE_SOURCE_DIR}/kget/core/scheduler.cpp
   ${CMAKE_SOURCE_DIR}/kget/core/transfer.cpp
   ${CMAKE_SOURCE_DIR}/kget/core/transfergroup.cpp
   ${CMAKE_SOURCE_DIR}/kget/core/transfergrouphandler.cpp
   ${CMAKE_SOURCE_DIR}/kget/core/transferhandler.cpp
   ${CMAKE_SOURCE_DIR}/kget/core/plugin/plugin.cpp
   ${CMAKE_SOURCE_DIR}/kget/core/plugin/transferfactory.cpp
)

set(kgetcore_SRCS ${kgetconf_SRCS} ${kgetcore_SRCS})

kde4_add_kcfg_files(kgetcore_SRCS ${CMAKE_SOURCE_DIR}/kget/conf/settings.kcfgc)

kde4_automoc(${kgetcore_SRCS})

kde4_add_library(kgetcore SHARED ${kgetcore_SRCS})

target_link_libraries(kgetcore ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS})

install_targets(${LIB_INSTALL_DIR} kgetcore)


set(kget_SRCS
   ${CMAKE_SOURCE_DIR}/kget/ui/droptarget.cpp
   ${CMAKE_SOURCE_DIR}/kget/ui/mainview.cpp
#    ${CMAKE_SOURCE_DIR}/kget/ui/sidebar.cpp
   ${CMAKE_SOURCE_DIR}/kget/ui/splash.cpp
   ${CMAKE_SOURCE_DIR}/kget/ui/transferdetails.cpp
   ${CMAKE_SOURCE_DIR}/kget/ui/tray.cpp
#    ${CMAKE_SOURCE_DIR}/kget/ui/traytransfer.cpp
   ${CMAKE_SOURCE_DIR}/kget/ui/viewscontainer.cpp
   ${CMAKE_SOURCE_DIR}/kget/kget.cpp
   ${CMAKE_SOURCE_DIR}/kget/main.cpp
)

qt4_add_dbus_adaptor(kget_SRCS org.kde.kget.xml kgetadaptor.h KGetAdaptor)

kde4_add_ui_files(kget_SRCS
   ${CMAKE_SOURCE_DIR}/kget/ui/transferdetailsfrm.ui
)

kde4_automoc(${kget_SRCS})

kde4_add_executable(kget ${kget_SRCS})

target_link_libraries(kget ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBRARY} ${KDE4_KIO_LIBRARY} kgetcore)

install_targets(/bin kget)

########### install files ###############

install_files(${XDG_APPS_DIR} FILES kget.desktop)
install_files(${DATA_INSTALL_DIR}/kget FILES kgetui.rc)

kde4_install_icons(${ICON_INSTALL_DIR} hicolor)
