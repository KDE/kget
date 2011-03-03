set(verifiertest_SRCS
    verifiertest.cpp
)

kde4_add_unit_test(verifiertest
  TESTNAME kget-verifiertest
  ${verifiertest_SRCS}
)

target_link_libraries(verifiertest kgetcore ${KDE4_KDECORE_LIBS} ${KDE4_KIO_LIBS} ${QT_QTTEST_LIBRARY})

if (QCA2_FOUND)
    target_link_libraries(verifiertest ${QCA2_LIBRARIES})
endif (QCA2_FOUND)

if (QGPGME_FOUND)
    target_link_libraries(verifiertest ${QGPGME_LIBRARIES})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")
endif(QGPGME_FOUND)
