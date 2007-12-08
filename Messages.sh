#! /bin/sh
$EXTRACTRC `find . -name \*.ui` >> rc.cpp || exit 11
$EXTRACTRC `find . -name \*.rc` >> rc.cpp || exit 11
$XGETTEXT `find . -name \*.cpp \*.h`  -o $podir/kget.pot
rm -f rc.cpp
