kde4_header()

include_directories(
   ${CMAKE_SOURCE_DIR}/kget/
   ${CMAKE_BINARY_DIR}/kget/
   ${KDE4_INCLUDE_DIR}
   ${QT_INCLUDES}
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_BINARY_DIR}
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

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kget_multithreadedfactory)

target_link_libraries(kget_multithreadedfactory ${QT_QTNETWORK_LIBRARY} ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} kgetcore)

install_targets(${LIB_INSTALL_DIR}/kde4 kget_multithreadedfactory)

########### install files ###############

install_files(${SERVICES_INSTALL_DIR} FILES kget_multithreadedfactory.desktop)

kde4_footer()

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
