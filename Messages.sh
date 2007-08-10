#! /bin/sh
$EXTRACTRC */*.ui > rc.cpp || exit 11
$EXTRACTRC *.rc */*.rc >> rc.cpp || exit 11
$XGETTEXT *.cpp */*.cpp  -o $podir/kget.pot
rm -f rc.cpp
