cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

# KDE Application Version, managed by release script
set(RELEASE_SERVICE_VERSION_MAJOR "23")
set(RELEASE_SERVICE_VERSION_MINOR "03")
set(RELEASE_SERVICE_VERSION_MICRO "90")
set(RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")

project(kget VERSION ${RELEASE_SERVICE_VERSION})

set(REQUIRED_QT_VERSION "5.15.0")
set(KF5_VERSION "5.91.0")
set(KDE_COMPILERSETTINGS_LEVEL "5.82")

find_package(ECM ${KF5_VERSION} REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(ECMAddAppIcon)
include(ECMInstallIcons)
include(ECMAddTests)
include(ECMQtDeclareLoggingCategory)
include(ECMSetupVersion)
include(ECMSetupQtPluginMacroNames)
include(GenerateExportHeader)
include(KDEClangFormat)
include(KDEGitCommitHooks)

find_package(Qt${QT_MAJOR_VERSION} ${REQUIRED_QT_VERSION} REQUIRED CONFIG COMPONENTS
    DBus
    Gui
    Network
    Sql
    Test
    Widgets
    Xml
)

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
    TextWidgets
    Wallet
    WidgetsAddons
    WindowSystem
    XmlGui
)

#include(CheckIncludeFile)
#include(CheckIncludeFiles)
#include(CheckSymbolExists)
#include(CheckFunctionExists)
#include(CheckLibraryExists)
#include(CheckPrototypeExists)
#include(CheckTypeSize)

if(WIN32)
   set(CMAKE_REQUIRED_LIBRARIES ${KDEWIN32_LIBRARIES})
   set(CMAKE_REQUIRED_INCLUDES  ${KDEWIN32_INCLUDES})
endif()

find_package(SQLite3)
set_package_properties(SQLite3 PROPERTIES
    DESCRIPTION "SQLite is a Binary-Database"
    TYPE RECOMMENDED
    PURPOSE "Needed for the SQLite-Backend of the KGet-History"
)

find_package(Qca-qt${QT_MAJOR_VERSION} 2.1.0)
set_package_properties(Qca-qt${QT_MAJOR_VERSION} PROPERTIES
    DESCRIPTION "Support for encryption"
    URL "https://download.kde.org/stable/qca-qt5/"
    TYPE REQUIRED
)

find_package(Gpgmepp 1.7.0)
set_package_properties(Gpgmepp PROPERTIES
    DESCRIPTION "The GpgME++ library"
    URL "https://www.gnupg.org"
    TYPE RECOMMENDED
    PURPOSE "GpgME++ is required to have signature verifying support in KGet."
)

find_package(KF5Torrent 21.07.70)
set_package_properties(KF5Torrent PROPERTIES
    DESCRIPTION "Backend Library of KTorrent"
    TYPE RECOMMENDED
    PURPOSE "Needed to build KGet bittorrent support."
)

find_package(LibMms)
set_package_properties(LibMms PROPERTIES
    DESCRIPTION "Library to read mms streams"
    TYPE RECOMMENDED
    PURPOSE "Needed to build KGet mms support."
)

if(CMAKE_BUILD_TYPE MATCHES debugfull)
     add_definitions(-DDEBUG)
endif()

if (Qca-qt${QT_MAJOR_VERSION}_FOUND)
    add_definitions(-DHAVE_QCA2)
MESSAGE(STATUS "QCA found")
endif()

find_package(Boost REQUIRED)

if(Gpgmepp_FOUND)
    if (QT_MAJOR_VERSION STREQUAL "6")
        find_package(QGpgmeQt6 ${GPGME_REQUIRED_VERSION} CONFIG REQUIRED)
    else()
        find_package(QGpgme ${GPGME_REQUIRED_VERSION} CONFIG REQUIRED)
    endif()
    add_definitions(-DHAVE_QGPGME)
endif()

set (KGET_PLUGIN_INSTALL_DIR ${KDE_INSTALL_PLUGINDIR}/kget)

add_definitions(-DQT_NO_URL_CAST_FROM_STRING)

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})

ecm_setup_version(PROJECT
    VARIABLE_PREFIX KGET
    VERSION_HEADER "kget_version.h"
)

ecm_setup_qtplugin_macro_names(JSON_NONE KGET_EXPORT_PLUGIN KGET_EXPORT_PLUGIN_CONFIG)

add_subdirectory(conf)
add_subdirectory(core)
add_subdirectory(ui)
add_subdirectory(transfer-plugins)
add_subdirectory(sounds)
add_subdirectory(desktop)
add_subdirectory(tests)
add_subdirectory(doc)

find_package(LibKWorkspace CONFIG)
set_package_properties(LibKWorkspace PROPERTIES
    TYPE OPTIONAL
    PURPOSE "Allows 'shutdown after downloads completed' in kget"
)
if(LibKWorkspace_FOUND)
    add_definitions(-DHAVE_KWORKSPACE)
endif()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/core/
    ${CMAKE_CURRENT_BINARY_DIR}/core/
)

if(SQLite3_FOUND)
    add_definitions(-DHAVE_SQLITE)
endif()

remove_definitions(-DQT_NO_HTTP)

# kgetcore

add_library(kgetcore SHARED)
set_target_properties(kgetcore PROPERTIES
    VERSION 5.0.0
    SOVERSION 5
)

set(kgetcore_dbus_SRCS)
qt_add_dbus_adaptor(kgetcore_dbus_SRCS dbus/org.kde.kget.transfer.xml dbus/dbustransferwrapper.h DBusTransferWrapper)
qt_add_dbus_adaptor(kgetcore_dbus_SRCS dbus/org.kde.kget.verifier.xml dbus/dbusverifierwrapper.h DBusVerifierWrapper)

target_sources(kgetcore PRIVATE
    ${kgetcore_dbus_SRCS}
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

ecm_qt_declare_logging_category(kgetcore
    HEADER kget_debug.h
    IDENTIFIER KGET_DEBUG
    CATEGORY_NAME kget
    DESCRIPTION "KGet"
    EXPORT KGET
)

kconfig_add_kcfg_files(kgetcore conf/settings.kcfgc)

generate_export_header(kgetcore BASE_NAME kget)

target_link_libraries(kgetcore
    KF5::ConfigGui
    KF5::CoreAddons
    KF5::DBusAddons
    KF5::I18n
    KF5::I18nLocaleData
    KF5::IconThemes
    KF5::KIOCore
    KF5::KIOWidgets
    KF5::Notifications
    KF5::XmlGui
    Qt::Core
    Qt::Widgets
    Qt::Sql
    Qt::Network
    Qt::DBus
    Boost::headers
)

if(LibKWorkspace_FOUND)
    target_link_libraries(kgetcore PW::KWorkspace)
endif()

if (SQLITE_FOUND)
    target_link_libraries(kgetcore ${QT_QTSQL_LIBRARY})
endif()

if (Qca-qt${QT_MAJOR_VERSION}_FOUND)
    target_link_libraries(kgetcore qca-qt${QT_MAJOR_VERSION})
endif ()

if (Gpgmepp_FOUND)
    target_link_libraries(kgetcore QGpgme Gpgmepp)
    kde_enable_exceptions()
endif()

install(TARGETS kgetcore ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})


# kget

add_executable(kget)

set(kget_dbus_SRCS)
qt_add_dbus_adaptor(kget_dbus_SRCS dbus/org.kde.kget.main.xml dbus/dbuskgetwrapper.h DBusKGetWrapper)

target_sources(kget PRIVATE
    ${kget_dbus_SRCS}
#     core/transferhandler.cpp
#     core/transfertreemodel.cpp
    dbus/dbuskgetwrapper.cpp
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

ki18n_wrap_ui(kget
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

file(GLOB ICON_SRCS "ui/icons/*-apps-kget.png")
ecm_add_app_icon(kget ICONS ${ICON_SRCS})

target_link_libraries(kget
    kgetcore
    KF5::Crash
    KF5::Wallet
    KF5::DBusAddons
    KF5::I18n
    KF5::Completion
    KF5::CoreAddons
    KF5::ItemViews
    KF5::KCMUtils
    KF5::NotifyConfig
    KF5::WindowSystem
    Qt::Core
    Qt::Widgets
    Qt::Sql
    Qt::Network
    Qt::DBus
)
if(Qca_FOUND)
    target_link_libraries(kget qca)
endif()
if(CMAKE_BUILD_TYPE MATCHES debugfull)
    target_link_libraries(kget Qt::Test)
endif()

if (Gpgmepp_FOUND)
    if (QT_MAJOR_VERSION STREQUAL "6")
        target_link_libraries(kget QGpgmeQt6)
    else()
        target_link_libraries(kget QGpgme)
    endif()
endif()

install(TARGETS kget ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

ecm_qt_install_logging_categories(
    EXPORT KGET
    FILE kget.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)
install(FILES org.kde.kget.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})

# add clang-format target for all our real source files
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

ki18n_install(po)
kdoctools_install(po)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
