kde4_header()

include_directories(
   ${CMAKE_SOURCE_DIR}/kget/
   ${KDE4_INCLUDE_DIR}
   ${QT_INCLUDES}
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_BINARY_DIR}
)

########### next target ###############

set(kget_kiofactory_PART_SRCS
  transferKio.cpp
  transferKioFactory.cpp
)

kde4_automoc(${kget_kiofactory_PART_SRCS})

kde4_add_plugin(kget_kiofactory WITH_PREFIX ${kget_kiofactory_PART_SRCS})

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kget_kiofactory)

target_link_libraries(kget_kiofactory ${KDE4_KDECORE_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KDEUI_LIBS} kgetcore)

install_targets(${LIB_INSTALL_DIR}/kde4 kget_kiofactory)


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES  kget_kiofactory.desktop )

kde4_footer()

#original Makefile.am contents follow:

#noinst_LTLIBRARIES = libtkio.la
#libtkio_la_SOURCES = transferKio.cpp
#libtkio_la_LIBADD = ../../core/libcore.la
#
### ---- KIO PLUGIN ----
### Create plugin library
#kde_module_LTLIBRARIES = libkget_kiofactory.la
#
#libkget_kiofactory_la_SOURCES = transferKioFactory.cpp
#libkget_kiofactory_la_LIBADD = $(LIB_QTCORE) $(LIB_QTGUI) $(LIB_QT3SUPPORT) $(LIB_KIO) $(LIB_KDEUI) $(LIB_KDECORE) libtkio.la ../../conf/libconf.la
#libkget_kiofactory_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
#
### Install plugin definition
#kde_services_DATA = kget_kiofactory.desktop
### --------------------
#
#
#INCLUDES = $(all_includes) -I$(srcdir)/../..
#
#METASOURCES = AUTO
