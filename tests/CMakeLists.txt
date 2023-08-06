include_directories(
   ../
)

#===========KGet===========
if(BUILD_TESTING)
    add_definitions(-DDO_KGET_TEST)
    add_executable(kget_test_transfers)
    set(kget_test_transfers_dbus_SRCS)
    qt_add_dbus_interface(kget_test_transfers_dbus_SRCS ../dbus/org.kde.kget.main.xml kget_interface)
    qt_add_dbus_interface(kget_test_transfers_dbus_SRCS ../dbus/org.kde.kget.transfer.xml transfer_interface)
    qt_add_dbus_interface(kget_test_transfers_dbus_SRCS ../dbus/org.kde.kget.verifier.xml verifier_interface)

    target_sources(kget_test_transfers PRIVATE
        ${kget_test_transfers_dbus_SRCS}
        testkget.cpp
        testtransfers.cpp
        ../kget_debug.cpp
    )

    # TODO enable it in ctest once it's fixed to not popup a "first run" messagebox on first start.
    # This probably needs a --testmode command-line argument (and QStandardPaths::setTestModeEnabled(true) on both sides)
    #ecm_mark_as_test(kget_test_transfers)
    target_link_libraries(kget_test_transfers Qt::Test KF5::KIOCore kgetcore)


#===========Verifier===========
    if (Qca-qt${QT_MAJOR_VERSION}_FOUND)

        ecm_add_test(
                verifiertest.cpp
            LINK_LIBRARIES
                Qt::Test
                ${QCA2_LIBRARIES}
                kgetcore
            TEST_NAME verifiertest)
    endif ()


    #===========Scheduler===========
    ecm_add_test(
            schedulertest.cpp
        LINK_LIBRARIES
            Qt::Test
            Qt::Gui
            kgetcore
        TEST_NAME schedulertest)

    #===========Metalinker===========
    ecm_add_test(
            metalinktest.cpp
            ../ui/metalinkcreator/metalinker.cpp
            ../kget_debug.cpp
        LINK_LIBRARIES
            Qt::Xml
            KF5::I18n
            KF5::KIOCore
            Qt::Test
        TEST_NAME metalinktest
    )

    #===========FileDeleter===========
    ecm_add_test(
            filedeletertest.cpp
            ../core/filedeleter.cpp
        LINK_LIBRARIES
            KF5::KIOCore
            Qt::Test
        TEST_NAME filedeletertest
    )

endif()
