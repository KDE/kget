if(KDE4_BUILD_TESTS)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. )

set(kiotest_SRCS 
    kiotest.cpp
    ../MultiSegKio.cpp
)

kde4_automoc(${kiotest_SRCS})

kde4_add_executable(kiotest NOGUI RUN_UNINSTALLED ${kiotest_SRCS})

target_link_libraries(kiotest ${KDE4_KIO_LIBS} ${QT_QTTEST_LIBRARY})

endif(KDE4_BUILD_TESTS)
