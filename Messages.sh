#! /bin/sh
$EXTRACTRC *.ui > rc.cpp || exit 11
$EXTRACTRC *.rc */*.rc >> rc.cpp || exit 11
$XGETTEXT *.cpp */*.cpp *.h -o $podir/kget.pot
