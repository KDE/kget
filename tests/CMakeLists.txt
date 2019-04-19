include_directories(
   ../
)

#===========KGet===========
if(CMAKE_BUILD_TYPE MATCHES debugfull)
    ENABLE_TESTING()

    add_definitions(-DDO_KGET_TEST)
    set(kget_test_transfers_SRCS
        testkget.cpp
        testtransfers.cpp
        ../kget_debug.cpp
    )

    qt5_add_dbus_interface(kget_test_transfers_SRCS ../dbus/org.kde.kget.main.xml kget_interface)
    qt5_add_dbus_interface(kget_test_transfers_SRCS ../dbus/org.kde.kget.transfer.xml transfer_interface)
    qt5_add_dbus_interface(kget_test_transfers_SRCS ../dbus/org.kde.kget.verifier.xml verifier_interface)

    #REACTIVATE
    #add_executable(kget_test_transfers ${kget_test_transfers_SRCS})
    #add_test(kget_test_transfers kget_test_transfers)
    #ecm_mark_as_test(kget_test_transfers)

    #target_link_libraries(kget_test_transfers Qt5::Test KF5::KIOCore kgetcore)


#===========Verifier===========
    if (QCA2_FOUND)
        add_executable(verifiertest verifiertest.cpp)
        add_test(verifiertest verifiertest)
        ecm_mark_as_test(verifiertest)

        target_link_libraries(verifiertest Qt5::Test ${QCA2_LIBRARIES} kgetcore)
    endif (QCA2_FOUND)


    #===========Scheduler===========
    add_executable(schedulertest schedulertest.cpp)
    add_test(schedulertest schedulertest ) 
    ecm_mark_as_test(schedulertest)

    target_link_libraries(schedulertest Qt5::Test Qt5::Gui kgetcore)


    #===========Metalinker===========
    set(metalinktest_SRCS
        metalinktest.cpp
        ../ui/metalinkcreator/metalinker.cpp
        ../kget_debug.cpp
    )

    add_executable( metalinktest ${metalinktest_SRCS})
    add_test(metalinktest metalinktest)
    ecm_mark_as_test(metalinktest)

    target_link_libraries(metalinktest Qt5::Xml KF5::I18n KF5::KIOCore Qt5::Test)

    #===========FileDeleter===========
    set(filedeletertest_SRCS
        filedeletertest.cpp
        ../core/filedeleter.cpp
    )

    add_executable(filedeletertest ${filedeletertest_SRCS})
     add_test(filedeletertest filedeletertest)
     ecm_mark_as_test(filedeletertest)

    target_link_libraries(filedeletertest KF5::KIOCore Qt5::Test)

endif(CMAKE_BUILD_TYPE MATCHES debugfull)
