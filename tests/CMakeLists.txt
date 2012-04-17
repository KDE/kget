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
    )

    qt4_add_dbus_interface(kget_test_transfers_SRCS ../dbus/org.kde.kget.main.xml kget_interface)
    qt4_add_dbus_interface(kget_test_transfers_SRCS ../dbus/org.kde.kget.transfer.xml transfer_interface)
    qt4_add_dbus_interface(kget_test_transfers_SRCS ../dbus/org.kde.kget.verifier.xml verifier_interface)

    kde4_add_unit_test(kget_test_transfers
        ${kget_test_transfers_SRCS})

    target_link_libraries(kget_test_transfers ${QT_QTTEST_LIBRARY} ${KDE4_KIO_LIBS} kgetcore)


    #===========Verifier===========
    if (QCA2_FOUND)
        kde4_add_unit_test(verifiertest
        TESTNAME kget-verifiertest
        verifiertest.cpp
        )

        target_link_libraries(verifiertest ${QT_QTTEST_LIBRARY} ${KDE4_KDECORE_LIBS} ${QCA2_LIBRARIES} kgetcore)
    endif (QCA2_FOUND)


    #===========Scheduler===========
    kde4_add_unit_test(schedulertest
    TESTNAME kget-schedulertest
    schedulertest.cpp
    )

    target_link_libraries(schedulertest ${QT_QTTEST_LIBRARY} ${QT_QTGUI_LIBRARY} ${KDE4_KDECORE_LIBS} kgetcore)


    #===========Metalinker===========
    set(metalinktest_SRCS
        metalinktest.cpp
        ../ui/metalinkcreator/metalinker.cpp
    )

    kde4_add_unit_test( metalinktest
    TESTNAME kget-metalinktest
    ${metalinktest_SRCS}
    )

    target_link_libraries(metalinktest ${KDE4_KDECORE_LIBS} ${KDE4_KIO_LIBS} ${QT_QTTEST_LIBRARY})

    if (HAVE_NEPOMUK)
    target_link_libraries(metalinktest ${SOPRANO_LIBRARIES} ${NEPOMUK_LIBRARIES})
    endif (HAVE_NEPOMUK)

    #===========FileDeleter===========
    set(filedeletertest_SRCS
        filedeletertest.cpp
        ../core/filedeleter.cpp
    )

    kde4_add_unit_test(filedeletertest
        TESTNAME kget-filedeletertest
        ${filedeletertest_SRCS}
    )

    target_link_libraries(filedeletertest ${KDE4_KIO_LIBS} ${QT_QTTEST_LIBRARY})

endif(CMAKE_BUILD_TYPE MATCHES debugfull)
