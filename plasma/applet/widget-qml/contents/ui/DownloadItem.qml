/*
 *   Copyright 2013 Bhushan Shah <bhush94@gmail.com>
 *   Copyright 2012-2013 Daniel Nicoletti <dantti12@gmail.com>
 *   Copyright 2013 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as Components
import org.kde.qtextracomponents 0.1

Item {
    id: downloadItem
    clip: true
    width: parent.width
    height: expanded ? downloadInfos.height + padding.margins.top + padding.margins.bottom * 2 + actionRow.height
                     : downloadInfos.height + padding.margins.top + padding.margins.bottom

    Behavior on height { PropertyAnimation {} }

    property bool expanded
    property bool highlight

    function updateSelection() {
        var containsMouse = mouseArea.containsMouse;

        if (highlight || expanded && containsMouse) {
            padding.opacity = 1;
        } else if (expanded) {
            padding.opacity = 0.8;
        } else if (containsMouse) {
            padding.opacity = 0.65;
        } else {
            padding.opacity = 0;
        }
    }

    PlasmaCore.FrameSvgItem {
        id: padding
        imagePath: "widgets/viewitem"
        prefix: "hover"
        opacity: 0
        Behavior on opacity { PropertyAnimation {} }
        anchors.fill: parent
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onEntered: updateSelection()
        onExited: updateSelection()
        onClicked: {
            if (expanded) {
                expanded = false;
            } else {
                expanded = true;
                downloadItem.forceActiveFocus();
            }
            updateSelection();
        }
    }

    Item {
        id: downloadInfos
        height: Math.max(downloadIcon.height,downloadNameLabel.height + downloadPercentBar.height)

        anchors {
            top: parent.top
            topMargin: padding.margins.top
            left: parent.left
            leftMargin: padding.margins.left
            right: parent.right
            rightMargin: padding.margins.right
        }

        QIconItem {
            id: downloadIcon
            width: theme.iconSizes.dialog
            height: width
            opacity: model["status"]=="Stopped"? 0.5 : 1
            Behavior on opacity { PropertyAnimation {} }
            anchors {
                verticalCenter: parent.verticalCenter
                left: parent.left
            }
            icon: model["icon"]
        }

        Components.Label {
            id: downloadNameLabel
            anchors {
                verticalCenter: undefined
                top: parent.top
                left: downloadIcon.right
                leftMargin: 6
            }
            height: paintedHeight
            elide: Text.ElideRight
            text: model["filename"]
        }

        Components.Label {
            id: downloadStatusLabel
            anchors {
                top: downloadNameLabel.top
                left: downloadNameLabel.right
                leftMargin: 3
            }
            text: model["status"]
            height: paintedHeight
            visible: true
            color: "#77"+(theme.textColor.toString().substr(1))
        }

        Components.ProgressBar {
            id: downloadPercentBar
            anchors {
                bottom: parent.bottom
                left: downloadIcon.right
                leftMargin: 6
                right: downloadPercent.left
                rightMargin: 6
            }
            minimumValue: 0
            maximumValue: 100
            visible: true
            value: model["progress"]
        }

        Components.Label {
            id: downloadPercent
            anchors {
                verticalCenter: downloadPercentBar.verticalCenter
                right: parent.right
            }
            visible: true
            text: model["progress"]+"%"
        }
    }

    Column {
        id: actionRow
        opacity: expanded ? 1 : 0
        width: parent.width
        anchors {
          top: downloadInfos.bottom
          topMargin: padding.margins.bottom
          left: parent.left
          leftMargin: padding.margins.left
          right: parent.right
          rightMargin: padding.margins.right
          bottomMargin: padding.margins.bottom
        }
        spacing: 4
        Behavior on opacity { PropertyAnimation {} }

        PlasmaCore.SvgItem {
            svg: PlasmaCore.Svg {
                id: lineSvg
                imagePath: "widgets/line"
            }
            elementId: "horizontal-line"
            height: lineSvg.elementSize("horizontal-line").height
            width: parent.width
        }

        Row {
            id: detailsRow
            width: parent.width
            spacing: 4

            Column {
                id: labelsColumn
                Components.Label {
                    height: paintedHeight
                    width: parent.width
                    horizontalAlignment: Text.AlignRight
                    onPaintedWidthChanged: {
                        if (paintedWidth > parent.width) { parent.width = paintedWidth; }
                    }
                    text: i18n("Destination:")
                    visible: true
                    font.pointSize: theme.smallestFont.pointSize
                    color: "#99"+(theme.textColor.toString().substr(1))
                }
                Components.Label {
                    height: paintedHeight
                    width: parent.width
                    horizontalAlignment: Text.AlignRight
                    onPaintedWidthChanged: {
                        if (paintedWidth > parent.width) { parent.width = paintedWidth; }
                    }
                    text: i18n("Source:")
                    visible: true
                    font.pointSize: theme.smallestFont.pointSize
                    color: "#99"+(theme.textColor.toString().substr(1))
                }
                Components.Label {
                    height: paintedHeight
                    width: parent.width
                    horizontalAlignment: Text.AlignRight
                    onPaintedWidthChanged: {
                        if (paintedWidth > parent.width) { parent.width = paintedWidth; }
                    }
                    text: i18n("Speed:")
                    visible: true
                    font.pointSize: theme.smallestFont.pointSize
                    color: "#99"+(theme.textColor.toString().substr(1))
                }
            }
            Column {
                width: parent.width - labelsColumn.width - parent.spacing * 2
                Components.Label { // Destination file
                    id: destLabel
                    height: paintedHeight
                    width: parent.width
                    elide: Text.ElideRight
                    text: model["dest"]
                    visible: true
                    font.pointSize: theme.smallestFont.pointSize
                    color: "#99"+(theme.textColor.toString().substr(1))
                }
                Components.Label { // Source URL
                    id: sourceLabel
                    height: paintedHeight
                    width: parent.width
                    elide: Text.ElideRight
                    text: model["src"]
                    visible: true
                    font.pointSize: theme.smallestFont.pointSize
                    color: "#99"+(theme.textColor.toString().substr(1))
                }
                Components.Label { // Download Speed
                    id: vendorLabel
                    height: paintedHeight
                    width: parent.width
                    elide: Text.ElideRight
                    text: model["speed"]
                    visible: true
                    font.pointSize: theme.smallestFont.pointSize
                    color: "#99"+(theme.textColor.toString().substr(1))
                }
            }
        }
    }
}

