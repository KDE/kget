cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

# KDE Application Version, managed by release script
set(RELEASE_SERVICE_VERSION_MAJOR "24")
set(RELEASE_SERVICE_VERSION_MINOR "11")
set(RELEASE_SERVICE_VERSION_MICRO "80")
set(RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")

project(kget VERSION ${RELEASE_SERVICE_VERSION})

set(REQUIRED_QT_VERSION "6.5.0")
set(KF_VERSION "6.3.0")
set(KDE_COMPILERSETTINGS_LEVEL "5.82")

find_package(ECM ${KF_VERSION} REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(ECMAddAppIcon)
include(ECMInstallIcons)
include(ECMAddTests)
include(ECMQtDeclareLoggingCategory)
include(ECMSetupVersion)
include(GenerateExportHeader)
include(KDEClangFormat)
include(KDEGitCommitHooks)
include(ECMDeprecationSettings)

find_package(Qt6 ${REQUIRED_QT_VERSION} REQUIRED CONFIG COMPONENTS
    DBus
    Gui
    Network
    Sql
    Test
    Widgets
    Xml
)

find_package(KF6 ${KF_VERSION} REQUIRED
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
    StatusNotifierItem
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

if (NOT WIN32)
    find_package(Gpgmepp 1.7.0)
    set_package_properties(Gpgmepp PROPERTIES
        DESCRIPTION "The GpgME++ library"
        URL "https://www.gnupg.org"
        TYPE RECOMMENDED
        PURPOSE "GpgME++ is required to have signature verifying support in KGet."
    )

    find_package(QGpgmeQt6 1.7.0 CONFIG)
    set_package_properties(Gpgmepp PROPERTIES
        DESCRIPTION "QGpgME Library"
        URL "https://www.gnupg.org"
        TYPE OPTIONAL
        PURPOSE "QGpgME is required to have signature verifying support in KGet."
    )
endif()

find_package(KTorrent6 23.11.70)
set_package_properties(KTorrent6 PROPERTIES
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

find_package(Boost REQUIRED)

if (Gpgmepp_FOUND AND NOT WIN32)
    find_package(QGpgmeQt6 ${GPGME_REQUIRED_VERSION} CONFIG REQUIRED)
    add_definitions(-DHAVE_QGPGME)
endif()

set (KGET_PLUGIN_INSTALL_DIR ${KDE_INSTALL_PLUGINDIR}/kget)

add_definitions(-DQT_NO_URL_CAST_FROM_STRING)

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})

ecm_setup_version(PROJECT
    VARIABLE_PREFIX KGET
    VERSION_HEADER "kget_version.h"
)

ecm_set_disabled_deprecation_versions(
    KF 6.4.0
) 

ecm_qt_declare_logging_category(kget_debug_SRCS
    HEADER kget_debug.h
    IDENTIFIER KGET_DEBUG
    CATEGORY_NAME kget
    DESCRIPTION "KGet"
    EXPORT KGET
)

add_subdirectory(conf)
add_subdirectory(core)
add_subdirectory(ui)
add_subdirectory(transfer-plugins)
add_subdirectory(sounds)
add_subdirectory(desktop)
add_subdirectory(doc)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/core/
    ${CMAKE_CURRENT_BINARY_DIR}/core/
)

add_subdirectory(tests)

if(SQLite3_FOUND)
    add_definitions(-DHAVE_SQLITE)
endif()

remove_definitions(-DQT_NO_HTTP)


# kget

add_executable(kget)

set(kget_dbus_SRCS)
qt_add_dbus_adaptor(kget_dbus_SRCS dbus/org.kde.kget.main.xml dbus/dbuskgetwrapper.h DBusKGetWrapper)

target_sources(kget PRIVATE
    ${kget_dbus_SRCS}
#     core/transferhandler.cpp
#     core/transfertreemodel.cpp
    dbus/dbuskgetwrapper.cpp
    ${kget_debug_SRCS}
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

    ui/kgetui.qrc

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
    KF6::Crash
    KF6::Wallet
    KF6::DBusAddons
    KF6::I18n
    KF6::Completion
    KF6::CoreAddons
    KF6::ItemViews
    KF6::KCMUtils
    KF6::NotifyConfig
    KF6::WindowSystem
    Qt::Core
    Qt::Widgets
    Qt::Sql
    Qt::Network
    Qt::DBus
)
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
