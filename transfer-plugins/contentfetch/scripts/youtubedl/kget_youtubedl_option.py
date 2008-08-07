# This file is part of the KDE project
#
#  Copyright (C) 2008 Ningyu Shi <shiningyu@gmail.com>

#  This program is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public
#  License as published by the Free Software Foundation; either
#  version 2 of the License, or (at your option) any later version.

from ui_youtube_option import Ui_YoutubeDlOption
from PyQt4.QtCore import *
from PyQt4.QtGui import *

class YoutubeDlOptionWidget(QWidget):
    def __init__(self, kconfig):
        QWidget.__init__(self)
        self.ui = Ui_YoutubeDlOption()
        self.ui.setupUi(self)
        self.parser = kconfig
        self.cfgfile = 'kget_youtubedl.rc'
        self.loadSetting()
        self.connect(self.ui.useNetrcCheck, SIGNAL("stateChanged(int)"), self, SLOT("slotUseNetrcEnabled(int)"))

    def loadSetting(self):
        self.parser.setFile(self.cfgfile)
        self.ui.fileNameComboBox.setCurrentIndex(self.parser.read('FileSetting', 'UseLiteralName', 0))
        self.ui.qualityComboBox.setCurrentIndex(self.parser.read('FileSetting', 'Quality', 0))
        self.ui.loginGroupBox.setChecked(bool(self.parser.read('LoginInfo', 'Enabled', 0)))
        self.ui.usernameEdit.setText(self.parser.read('LoginInfo', 'Username', 'Nobody'))
        self.ui.passwordEdit.setText(self.parser.read('LoginInfo', 'Password', 'passwd'))
        self.ui.useNetrcCheck.setCheckState(Qt.CheckState(self.parser.read('LoginInfo', 'UseNetrc', 0)))
        if self.ui.useNetrcCheck.checkState() == Qt.Checked:
            self.enableUserPass(False)

    def saveSetting(self):
        self.parser.write('FileSetting', 'UseLiteralName', self.ui.fileNameComboBox.currentIndex())
        self.parser.write('FileSetting', 'Quality', self.ui.qualityComboBox.currentIndex())
        self.parser.write('LoginInfo', 'Enabled', int(self.ui.loginGroupBox.isChecked()))
        # kross seems not directly support Qt.QString, use python string instead
        self.parser.write('LoginInfo', 'Username', str(self.ui.usernameEdit.text()))
        self.parser.write('LoginInfo', 'Password', str(self.ui.passwordEdit.text()))
        self.parser.write('LoginInfo', 'UseNetrc', int(self.ui.useNetrcCheck.checkState()))

    def enableUserPass(self, state):
        self.ui.usernameLabel.setEnabled(state)
        self.ui.usernameEdit.setEnabled(state)
        self.ui.passwordLabel.setEnabled(state)
        self.ui.passwordEdit.setEnabled(state)
    # slot
    @pyqtSignature("int")
    def slotUseNetrcEnabled(self, state):
        if state == int(Qt.Checked):
            self.enableUserPass(False)
        else:
            self.enableUserPass(True)
