include_directories(
   ${CMAKE_SOURCE_DIR}/kget/
   ${CMAKE_BINARY_DIR}/kget/
   ${KDE4_INCLUDES}
   ${QT_INCLUDES}
)

########### next target ###############

set(kget_multithreadedfactory_SRCS
   mtdetailswidget.cpp
   mthreaded.cpp
   thread.cpp
   transferMultiThreaded.cpp
   transferMultiThreadedFactory.cpp
)

kde4_automoc(${kget_multithreadedfactory_SRCS})

kde4_add_ui_files(kget_multithreadedfactory_SRCS
   mtdetailswidgetfrm.ui
)

kde4_add_plugin(kget_multithreadedfactory WITH_PREFIX ${kget_multithreadedfactory_SRCS})

target_link_libraries(kget_multithreadedfactory ${QT_QTNETWORK_LIBRARY} ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} kgetcore)

install(TARGETS kget_multithreadedfactory DESTINATION ${LIB_INSTALL_DIR}/kde4)

########### install files ###############

install(FILES kget_multithreadedfactory.desktop DESTINATION ${SERVICES_INSTALL_DIR})
