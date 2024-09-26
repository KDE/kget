# kgetcore

add_library(kgetcore SHARED)
set_target_properties(kgetcore PROPERTIES
    VERSION 6.0.0
    SOVERSION 6
)

set(kgetcore_dbus_SRCS)
qt_add_dbus_adaptor(kgetcore_dbus_SRCS ../dbus/org.kde.kget.transfer.xml ../dbus/dbustransferwrapper.h DBusTransferWrapper)
qt_add_dbus_adaptor(kgetcore_dbus_SRCS ../dbus/org.kde.kget.verifier.xml ../dbus/dbusverifierwrapper.h DBusVerifierWrapper)

qt_add_dbus_interface(kgetcore_dbus_SRCS ../dbus/org.kde.Shutdown.xml plasmashudown_interface)

target_sources(kgetcore PRIVATE
    ${kgetcore_dbus_SRCS}
    ${kget_debug_SRCS}
    job.cpp
    jobqueue.cpp
    kget.cpp
    scheduler.cpp
    transfertreemodel.cpp
    transfertreeselectionmodel.cpp
    transfer.cpp
    transfergroup.cpp
    transfergrouphandler.cpp
    transferhandler.cpp
    handler.cpp
    transfergroupscheduler.cpp
    plugin/plugin.cpp
    plugin/transferfactory.cpp
    transferdatasource.cpp
    datasourcefactory.cpp
    kgetkjobadapter.cpp
    kuiserverjobs.cpp
    kgetglobaljob.cpp
    bitset.cpp
    download.cpp
    transferhistorystore.cpp
    transferhistorystore_xml.cpp
    transferhistorystore_sqlite.cpp
    linkimporter.cpp
    ../dbus/dbustransferwrapper.cpp
    ../dbus/dbusverifierwrapper.cpp
    filemodel.cpp
    verifier.cpp
    verificationthread.cpp
    verificationmodel.cpp
    verificationdelegate.cpp
    signature.cpp
    signaturethread.cpp
    keydownloader.cpp
    urlchecker.cpp
    basedialog.cpp
    mostlocalurl.cpp
    filedeleter.cpp
)

kconfig_add_kcfg_files(kgetcore ../conf/settings.kcfgc)

generate_export_header(kgetcore BASE_NAME kget)

target_link_libraries(kgetcore PUBLIC
    KF6::ConfigGui
    KF6::CoreAddons
    KF6::DBusAddons
    KF6::I18n
    KF6::I18nLocaleData
    KF6::IconThemes
    KF6::IconWidgets
    KF6::KCMUtils
    KF6::KIOCore
    KF6::KIOWidgets
    KF6::Notifications
    KF6::StatusNotifierItem
    KF6::XmlGui
    Qt6::Core
    Qt6::Widgets
    Qt6::Sql
    Qt6::Network
    Qt6::DBus
    Boost::headers
)

if (SQLITE_FOUND)
    target_link_libraries(kgetcore PUBLIC ${QT_QTSQL_LIBRARY})
endif()

if (TARGET Gpgmepp)
    target_link_libraries(kgetcore PUBLIC QGpgmeQt6 Gpgmepp)
    kde_enable_exceptions()
endif()

install(TARGETS kgetcore ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

