#! /bin/sh
kget_subdirs="conf core dbus transfer-plugins/kio transfer-plugins/metalink transfer-plugins/multisegmentkio transfer-plugins/bittorrent ui extensions/konqueror"
$EXTRACTRC `find $kget_subdirs -name \*.ui` >> rc.cpp || exit 11
$EXTRACTRC `find $kget_subdirs -name \*.rc` >> rc.cpp || exit 11
$XGETTEXT `find $kget_subdirs -name \*.cpp -o -name \*.h` *.cpp *.h -o $podir/kget.pot
rm -f rc.cpp
