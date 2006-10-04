include_directories(
   ${CMAKE_SOURCE_DIR}/kget/
   ${CMAKE_BINARY_DIR}/kget/
   ${KDE4_INCLUDE_DIR}
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

#original Makefile.am contents follow:

#
#kde_module_LTLIBRARIES = libkget_multithreadedfactory.la
#
#libkget_multithreadedfactory_la_SOURCES = transferMultiThreadedFactory.cpp transferMultiThreaded.cpp mthreaded.cpp thread.cpp mtdetailswidget.cpp mtdetailswidgetfrm.ui
#
#libkget_multithreadedfactory_la_LIBADD = $(LIB_QTCORE) $(LIB_QTGUI) $(LIB_QT3SUPPORT) $(LIB_KDEUI) $(LIB_KDECORE) $(LIB_KIO) ../../conf/libconf.la ../../core/libcore.la
#
#libkget_multithreadedfactory_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
#
#kde_services_DATA = kget_multithreadedfactory.desktop
#
#INCLUDES = $(all_includes) -I$(qt_includes)/QtNetwork -I$(srcdir)/../..
#
#METASOURCES = AUTO
