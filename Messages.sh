#! /bin/sh
kget_subdirs="conf core dbus transfer-plugins ui extensions/konqueror extensions/webinterface"
$EXTRACTRC `find $kget_subdirs -name \*.ui` >> rc.cpp || exit 11
$EXTRACTRC `find $kget_subdirs -name \*.rc` >> rc.cpp || exit 12
$XGETTEXT `find $kget_subdirs -name \*.cpp -o -name \*.h` *.cpp *.h -o $podir/kget.pot
rm -f rc.cpp
