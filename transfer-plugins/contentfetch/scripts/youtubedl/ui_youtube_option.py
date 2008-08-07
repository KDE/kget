# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'youtube_option.ui'
#
# Created: Tue Aug  5 22:51:51 2008
#      by: PyQt4 UI code generator 4.4.3-snapshot-20080804
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

class Ui_YoutubeDlOption(object):
    def setupUi(self, YoutubeDlOption):
        YoutubeDlOption.setObjectName("YoutubeDlOption")
        YoutubeDlOption.resize(395, 349)
        self.verticalLayout = QtGui.QVBoxLayout(YoutubeDlOption)
        self.verticalLayout.setObjectName("verticalLayout")
        self.fileSettingGroupBox = QtGui.QGroupBox(YoutubeDlOption)
        self.fileSettingGroupBox.setObjectName("fileSettingGroupBox")
        self.formLayout_3 = QtGui.QFormLayout(self.fileSettingGroupBox)
        self.formLayout_3.setFieldGrowthPolicy(QtGui.QFormLayout.AllNonFixedFieldsGrow)
        self.formLayout_3.setObjectName("formLayout_3")
        self.fileNameLabel = QtGui.QLabel(self.fileSettingGroupBox)
        self.fileNameLabel.setObjectName("fileNameLabel")
        self.formLayout_3.setWidget(0, QtGui.QFormLayout.LabelRole, self.fileNameLabel)
        self.fileNameComboBox = QtGui.QComboBox(self.fileSettingGroupBox)
        self.fileNameComboBox.setObjectName("fileNameComboBox")
        self.fileNameComboBox.addItem(QtCore.QString())
        self.fileNameComboBox.addItem(QtCore.QString())
        self.formLayout_3.setWidget(0, QtGui.QFormLayout.FieldRole, self.fileNameComboBox)
        self.qualityLabel = QtGui.QLabel(self.fileSettingGroupBox)
        self.qualityLabel.setObjectName("qualityLabel")
        self.formLayout_3.setWidget(1, QtGui.QFormLayout.LabelRole, self.qualityLabel)
        self.qualityComboBox = QtGui.QComboBox(self.fileSettingGroupBox)
        self.qualityComboBox.setObjectName("qualityComboBox")
        self.qualityComboBox.addItem(QtCore.QString())
        self.qualityComboBox.addItem(QtCore.QString())
        self.formLayout_3.setWidget(1, QtGui.QFormLayout.FieldRole, self.qualityComboBox)
        self.verticalLayout.addWidget(self.fileSettingGroupBox)
        self.loginGroupBox = QtGui.QGroupBox(YoutubeDlOption)
        self.loginGroupBox.setCheckable(True)
        self.loginGroupBox.setChecked(False)
        self.loginGroupBox.setObjectName("loginGroupBox")
        self.verticalLayout_2 = QtGui.QVBoxLayout(self.loginGroupBox)
        self.verticalLayout_2.setObjectName("verticalLayout_2")
        self.formLayout = QtGui.QFormLayout()
        self.formLayout.setObjectName("formLayout")
        self.passwordLabel = QtGui.QLabel(self.loginGroupBox)
        self.passwordLabel.setObjectName("passwordLabel")
        self.formLayout.setWidget(2, QtGui.QFormLayout.LabelRole, self.passwordLabel)
        self.passwordEdit = QtGui.QLineEdit(self.loginGroupBox)
        self.passwordEdit.setEchoMode(QtGui.QLineEdit.Password)
        self.passwordEdit.setObjectName("passwordEdit")
        self.formLayout.setWidget(2, QtGui.QFormLayout.FieldRole, self.passwordEdit)
        self.usernameLabel = QtGui.QLabel(self.loginGroupBox)
        self.usernameLabel.setObjectName("usernameLabel")
        self.formLayout.setWidget(1, QtGui.QFormLayout.LabelRole, self.usernameLabel)
        self.usernameEdit = QtGui.QLineEdit(self.loginGroupBox)
        self.usernameEdit.setObjectName("usernameEdit")
        self.formLayout.setWidget(1, QtGui.QFormLayout.FieldRole, self.usernameEdit)
        self.verticalLayout_2.addLayout(self.formLayout)
        self.useNetrcCheck = QtGui.QCheckBox(self.loginGroupBox)
        self.useNetrcCheck.setObjectName("useNetrcCheck")
        self.verticalLayout_2.addWidget(self.useNetrcCheck)
        self.verticalLayout.addWidget(self.loginGroupBox)
        spacerItem = QtGui.QSpacerItem(20, 40, QtGui.QSizePolicy.Minimum, QtGui.QSizePolicy.Expanding)
        self.verticalLayout.addItem(spacerItem)

        self.retranslateUi(YoutubeDlOption)
        QtCore.QMetaObject.connectSlotsByName(YoutubeDlOption)

    def retranslateUi(self, YoutubeDlOption):
        YoutubeDlOption.setWindowTitle(QtGui.QApplication.translate("YoutubeDlOption", "Form", None, QtGui.QApplication.UnicodeUTF8))
        self.fileSettingGroupBox.setTitle(QtGui.QApplication.translate("YoutubeDlOption", "File-Settings", None, QtGui.QApplication.UnicodeUTF8))
        self.fileNameLabel.setText(QtGui.QApplication.translate("YoutubeDlOption", "Filename:", None, QtGui.QApplication.UnicodeUTF8))
        self.fileNameComboBox.setItemText(0, QtGui.QApplication.translate("YoutubeDlOption", "Use normalized name", None, QtGui.QApplication.UnicodeUTF8))
        self.fileNameComboBox.setItemText(1, QtGui.QApplication.translate("YoutubeDlOption", "Use literal name", None, QtGui.QApplication.UnicodeUTF8))
        self.qualityLabel.setText(QtGui.QApplication.translate("YoutubeDlOption", "Quality:", None, QtGui.QApplication.UnicodeUTF8))
        self.qualityComboBox.setItemText(0, QtGui.QApplication.translate("YoutubeDlOption", "Best quality (.mp4)", None, QtGui.QApplication.UnicodeUTF8))
        self.qualityComboBox.setItemText(1, QtGui.QApplication.translate("YoutubeDlOption", "Normal quality (.flv)", None, QtGui.QApplication.UnicodeUTF8))
        self.loginGroupBox.setToolTip(QtGui.QApplication.translate("YoutubeDlOption", "Provide login info of your youtube account in order to access restricted media.", None, QtGui.QApplication.UnicodeUTF8))
        self.loginGroupBox.setTitle(QtGui.QApplication.translate("YoutubeDlOption", "Login Info", None, QtGui.QApplication.UnicodeUTF8))
        self.passwordLabel.setText(QtGui.QApplication.translate("YoutubeDlOption", "Password:", None, QtGui.QApplication.UnicodeUTF8))
        self.usernameLabel.setText(QtGui.QApplication.translate("YoutubeDlOption", "Username:", None, QtGui.QApplication.UnicodeUTF8))
        self.useNetrcCheck.setToolTip(QtGui.QApplication.translate("YoutubeDlOption", ".netrc must have a hostname called \'youtube\'.", None, QtGui.QApplication.UnicodeUTF8))
        self.useNetrcCheck.setText(QtGui.QApplication.translate("YoutubeDlOption", "User .netrc file", None, QtGui.QApplication.UnicodeUTF8))

