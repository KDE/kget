/*
    SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Dialogs 1.3
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.20 as Kirigami

Kirigami.Page {
    id: newTransferPage

    width: 400
    height: 246

    // For C++ side to get/set properties
    property bool multiple: false
    property string initialSource
    readonly property string source: multiple ? urlListRequester.text : urlRequester.text
    property alias destination: destRequester.text
    property alias groupCurrentIndex: groupComboBox.currentIndex
    property alias errorMessage: errorMessage.text

    title: i18nc("@label", "New Download")

    signal finished()
    signal rejected()

    /**
     * Emitted when the source/the destination/the group is changed
     */
    signal defaultDestinationChanged()

    /**
     * Emitted when the input needs check
     *
     * @see errorMessage
     */
    signal requestCheckInput()

    actions.main: Action {
        enabled: !inputTimer.running && !errorMessage.visible
        icon.name: "dialog-ok"
        text: i18nc("@action:button", "OK")
        onTriggered: newTransferPage.finished();
    }
    actions.right: Action {
        icon.name: "dialog-cancel"
        text: i18nc("@action:button", "Cancel")
        onTriggered: newTransferPage.rejected()
    }

    // Timer to avoid constant checking of the input
    Timer {
        id: inputTimer

        interval: 350

        onTriggered: newTransferPage.requestCheckInput();
    }

    FileDialog {
        id: fileDialog
    }

    Kirigami.FormLayout {
        id: gridLayout

        width: parent.width

        Kirigami.ActionTextField {
            id: urlRequester
            Kirigami.FormData.label: i18nc("@label:textbox", "URL:")
            visible: !newTransferPage.multiple

            text: multiple ? "" : initialSource
            rightActions: [
                Action {
                    icon.name: "edit-clear"
                    enabled: urlRequester.text.length > 0
                    text: i18nc("@action:button", "Reset URL")
                    onTriggered: urlRequester.clear();
                }
            ]

            onTextChanged: {
                inputTimer.restart();
                newTransferPage.defaultDestinationChanged();
            }
        }

        TextArea {
            id: urlListRequester
            Kirigami.FormData.label: i18nc("@label:textbox", "URL:")
            visible: newTransferPage.multiple

            text: multiple ? initialSource : ""

            onTextChanged: urlRequester.textChanged();
        }

        RowLayout {
            Kirigami.ActionTextField {
                id: destRequester
                Layout.fillWidth: true
                Kirigami.FormData.label: i18nc("@label:textbox", "Destination:")
                rightActions: [
                    Action {
                        icon.name: "edit-clear"
                        enabled: destRequester.text.length > 0
                        text: i18nc("@action:button", "Reset destination")
                        onTriggered: destRequester.clear();
                    }
                ]

                onTextChanged: inputTimer.restart();
            }

            Button {
                display: AbstractButton.IconOnly
                icon.name: "document-open"
                text: i18nc("@action:button", "Open file dialog")

                onClicked: fileDialog.open()
            }
        }

        ComboBox {
            id: groupComboBox
            Kirigami.FormData.label: i18nc("@label:combobox", "Transfer group:")

            onCurrentIndexChanged: newTransferPage.defaultDestinationChanged();
        }
    }

    Kirigami.InlineMessage {
        id: errorMessage

        anchors {
            top: gridLayout.bottom
            margins: Kirigami.Units.largeSpacing
            left: parent.left
            right: parent.right
        }
        visible: text.length > 0
    }
}
