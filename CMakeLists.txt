cmake_minimum_required(VERSION 3.0)

# KDE Application Version, managed by release script
set(RELEASE_SERVICE_VERSION_MAJOR "20")
set(RELEASE_SERVICE_VERSION_MINOR "07")
set(RELEASE_SERVICE_VERSION_MICRO "70")
set(RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")

project(kget VERSION ${RELEASE_SERVICE_VERSION})

set(KF5_VERSION "5.55.0")
set(REQUIRED_QT_VERSION "5.7.0")

find_package(ECM ${KF5_VERSION} REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} "${CMAKE_SOURCE_DIR}/cmake")

find_package(Qt5 ${REQUIRED_QT_VERSION} REQUIRED CONFIG COMPONENTS DBus Gui Network Sql Test Widgets Xml)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMInstallIcons)
include(ECMMarkAsTest)
include(GenerateExportHeader)
include(ECMQtDeclareLoggingCategory)
include(ECMSetupVersion)

ecm_setup_version(PROJECT VARIABLE_PREFIX KGET VERSION_HEADER "kget_version.h")

find_package(KF5 ${KF5_VERSION} REQUIRED
    Completion
    Config
    ConfigWidgets
    CoreAddons
    Crash
    DBusAddons
    DocTools
    I18n
    IconThemes
    ItemViews
    KCMUtils
    KIO
    Notifications
    NotifyConfig
    Parts
    Service
    TextWidgets
    Wallet
    WidgetsAddons
    WindowSystem
    XmlGui
    KDELibs4Support # KLocale
)

#include(CheckIncludeFile)
#include(CheckIncludeFiles)
#include(CheckSymbolExists)
#include(CheckFunctionExists)
#include(CheckLibraryExists)
#include(CheckPrototypeExists)
#include(CheckTypeSize)

#set(CMAKE_REQUIRED_DEFINITIONS ${_KDE4_PLATFORM_DEFINITIONS})
if(WIN32)
   set(CMAKE_REQUIRED_LIBRARIES ${KDEWIN32_LIBRARIES})
   set(CMAKE_REQUIRED_INCLUDES  ${KDEWIN32_INCLUDES})
endif(WIN32)

add_definitions(-DQT_NO_URL_CAST_FROM_STRING)
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})

find_package(Sqlite QUIET)
set_package_properties(SQLITE_FOUND PROPERTIES DESCRIPTION "SQLite is a Binary-Database" TYPE RECOMMENDED PURPOSE "Needed for the SQLite-Backend of the KGet-History")

find_package(Qca-qt5 2.1.0)
set_package_properties(Qca PROPERTIES DESCRIPTION "Support for encryption"
                       URL "https://download.kde.org/stable/qca-qt5/"
                       TYPE REQUIRED)


find_package(Gpgmepp 1.7.0)
set_package_properties(Gpgmepp_FOUND PROPERTIES DESCRIPTION "The GpgME++ library" URL "https://www.gnupg.org" TYPE RECOMMENDED PURPOSE "GpgME++ is required to have signature verifying support in KGet.")

find_package(KF5Torrent 2.1)
set_package_properties(KF5Torrent_FOUND PROPERTIES DESCRIPTION "Backend Library of KTorrent" TYPE RECOMMENDED PURPOSE "Needed to build KGet bittorrent support.")

find_package(LibMms QUIET)
set_package_properties(LIBMMS_FOUND PROPERTIES DESCRIPTION "Library to read mms streams" TYPE RECOMMENDED PURPOSE "Needed to build KGet mms support.")

set(kget_adaptor_SRCS
    dbus/dbuskgetwrapper.cpp
)
# set(kget_transfer_adaptor_SRCS
#     core/transferhandler.cpp
#     core/transfertreemodel.cpp
# )

#add_definitions(-DKDE_DEFAULT_DEBUG_AREA=5001)  //TODO: PORT THIS!!

if(CMAKE_BUILD_TYPE MATCHES debugfull)
     add_definitions(-DDEBUG)
endif(CMAKE_BUILD_TYPE MATCHES debugfull)

if (Qca-qt5_FOUND)
    add_definitions(-DHAVE_QCA2)
MESSAGE(STATUS "QCA found")
endif()

find_package(Boost REQUIRED)

if(Gpgmepp_FOUND)
    find_package(QGpgme REQUIRED)
    add_definitions(-DHAVE_QGPGME)
endif()

set (KGET_PLUGIN_INSTALL_DIR ${PLUGIN_INSTALL_DIR}/kget)

add_subdirectory(conf)
add_subdirectory(core)
add_subdirectory(ui)
add_subdirectory(transfer-plugins)
add_subdirectory(extensions)
add_subdirectory(sounds)
add_subdirectory(desktop)
#add_subdirectory(plasma) TODO PORT TO KF5/QML
add_subdirectory(tests)
add_subdirectory(doc)

find_package(LibKWorkspace CONFIG)
set_package_properties(LibKWorkspace PROPERTIES TYPE OPTIONAL PURPOSE "Allows 'shutdown after downloads completed' in kget")
if(LibKWorkspace_FOUND)
    add_definitions(-DHAVE_KWORKSPACE)
endif()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/core/
    ${CMAKE_CURRENT_BINARY_DIR}/core/
    ${CMAKE_CURRENT_BINARY_DIR}
)

if(SQLITE_FOUND)
    add_definitions(-DHAVE_SQLITE)
endif(SQLITE_FOUND)


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
   core/transferhistorystore_xml.cpp
   core/transferhistorystore_sqlite.cpp
   core/linkimporter.cpp
   dbus/dbustransferwrapper.cpp
   dbus/dbusverifierwrapper.cpp
   core/filemodel.cpp
   core/verifier.cpp
   core/verificationthread.cpp
   core/verificationmodel.cpp
   core/verificationdelegate.cpp
   core/signature.cpp
   core/signaturethread.cpp
   core/keydownloader.cpp
   core/urlchecker.cpp
   core/basedialog.cpp
   core/mostlocalurl.cpp
   core/filedeleter.cpp
)

ecm_qt_declare_logging_category(kgetcore_SRCS
                                HEADER kget_debug.h
                                IDENTIFIER KGET_DEBUG
                                CATEGORY_NAME kget)

kconfig_add_kcfg_files(kgetcore_SRCS conf/settings.kcfgc)

qt5_add_dbus_adaptor(kgetcore_SRCS dbus/org.kde.kget.transfer.xml dbus/dbustransferwrapper.h DBusTransferWrapper)
qt5_add_dbus_adaptor(kgetcore_SRCS dbus/org.kde.kget.verifier.xml dbus/dbusverifierwrapper.h DBusVerifierWrapper)


add_library(kgetcore SHARED ${kgetcore_SRCS})
generate_export_header(kgetcore BASE_NAME kget)

target_link_libraries(kgetcore Qt5::Core Qt5::Widgets Qt5::Sql Qt5::Network Qt5::DBus KF5::ConfigGui KF5::CoreAddons KF5::DBusAddons KF5::I18n KF5::IconThemes KF5::KIOCore KF5::KIOWidgets KF5::Notifications KF5::Service KF5::XmlGui)

if(LibKWorkspace_FOUND)
    target_link_libraries(kgetcore PW::KWorkspace)
endif()

if (SQLITE_FOUND)
  target_link_libraries(kgetcore ${QT_QTSQL_LIBRARY})
endif (SQLITE_FOUND)

if (Qca-qt5_FOUND)
    target_link_libraries(kgetcore qca-qt5)
endif ()

if (Gpgmepp_FOUND)
    target_link_libraries(kgetcore QGpgme Gpgmepp)
    kde_enable_exceptions()
endif()


set_target_properties(kgetcore PROPERTIES VERSION 5.0.0 SOVERSION 5 )
install(TARGETS kgetcore ${INSTALL_TARGETS_DEFAULT_ARGS})


# kget

set(kget_SRCS ${kget_adaptor_SRCS} ${kget_transfer_adaptor_SRCS}
   kget_debug.cpp
   conf/autopastemodel.cpp
   conf/integrationpreferences.cpp
   conf/dlgwebinterface.cpp
   conf/preferencesdialog.cpp
   conf/transfersgrouptree.cpp
   conf/transfersgroupwidget.cpp
   conf/pluginselector.cpp
   conf/verificationpreferences.cpp
   ui/droptarget.cpp
   ui/transfersview.cpp
   ui/transfersviewdelegate.cpp
   ui/transferdetails.cpp
   ui/viewscontainer.cpp
   ui/newtransferdialog.cpp
   ui/groupsettingsdialog.cpp
   ui/transfersettingsdialog.cpp
   ui/contextmenu.cpp
   ui/tray.cpp
   ui/history/rangetreewidget.cpp
   ui/history/transferhistory.cpp
   ui/history/transferhistoryitemdelegate.cpp
   ui/history/transferhistorycategorizeddelegate.cpp
   ui/history/transferhistorycategorizedview.cpp
   ui/linkview/kget_linkview.cpp
   ui/linkview/kget_sortfilterproxymodel.cpp
   ui/mirror/mirrorsettings.cpp
   ui/mirror/mirrormodel.cpp
   ui/renamefile.cpp
   ui/verificationdialog.cpp
   ui/metalinkcreator/metalinkcreator.cpp
   ui/metalinkcreator/generalwidget.cpp
   ui/metalinkcreator/metalinker.cpp
   ui/metalinkcreator/filedlg.cpp
   ui/metalinkcreator/localemodels.cpp
   ui/metalinkcreator/dragdlg.cpp
   ui/metalinkcreator/urlwidget.cpp
   ui/metalinkcreator/filehandler.cpp
   ui/signaturedlg.cpp
   mainwindow.cpp
   main.cpp

   #extensions/webinterface/httpserver.cpp
)

qt5_add_dbus_adaptor(kget_SRCS dbus/org.kde.kget.main.xml dbus/dbuskgetwrapper.h DBusKGetWrapper)

ki18n_wrap_ui(kget_SRCS
   conf/dlgadvanced.ui
   conf/dlgappearance.ui
   conf/dlggroups.ui
   conf/dlgintegration.ui
   conf/dlgwebinterface.ui
   conf/dlgnetwork.ui
   conf/verificationpreferences.ui
   ui/transferdetailsfrm.ui
   ui/newtransferwidget.ui
   ui/history/transferhistory.ui
   ui/groupsettingsdialog.ui
   ui/transfersettingsdialog.ui
   ui/linkview/importlinkdialog.ui
   ui/mirror/mirrorsettings.ui
   ui/mirror/mirroradddlg.ui
   ui/renamefile.ui
   ui/verificationdialog.ui
   ui/verificationadddlg.ui
   ui/metalinkcreator/introduction.ui
   ui/metalinkcreator/generalwidget.ui
   ui/metalinkcreator/files.ui
   ui/metalinkcreator/filedlg.ui
   ui/metalinkcreator/dragdlg.ui
   ui/metalinkcreator/urlwidget.ui
   ui/metalinkcreator/commondata.ui
   ui/signaturedlg.ui
)

#kde4_add_app_icon(kget_SRCS "ui/icons/hi*-apps-kget.png")

add_executable(kget ${kget_SRCS})

target_link_libraries(kget Qt5::Core Qt5::Widgets Qt5::Sql Qt5::Network Qt5::DBus KF5::Crash KF5::Wallet KF5::DBusAddons KF5::I18n KF5::Completion KF5::CoreAddons KF5::IconThemes KF5::ItemViews KF5::KCMUtils KF5::NotifyConfig KF5::WindowSystem KF5::KDELibs4Support kgetcore)
if(Qca_FOUND)
  target_link_libraries(kget qca)
endif()
if(CMAKE_BUILD_TYPE MATCHES debugfull)
  target_link_libraries(kget Qt5::Test)
endif()

if (QGpgme_FOUND)
  target_link_libraries(kget QGpgme)
endif()

install(TARGETS kget ${INSTALL_TARGETS_DEFAULT_ARGS})

if (${ECM_VERSION} STRGREATER "5.58.0")
    install(FILES kget.categories DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR})
else()
    install(FILES kget.categories DESTINATION ${KDE_INSTALL_CONFDIR})
endif()

install(FILES org.kde.kget.appdata.xml DESTINATION ${CMAKE_INSTALL_METAINFODIR})

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
