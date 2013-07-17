/*
 *   Copyright 2013 Bhushan Shah <bhush94@gmail.com>
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
    PlasmaCore.FrameSvgItem {
        id: padding
        imagePath: "widgets/viewitem"
        prefix: "hover"
        opacity: 0.8
        anchors.fill: parent
    }
    QIconItem {
        id: errorIcon
        width: height
        height: parent.height - errorLabel.paintedHeight - 5
        anchors {
            top: errorLabel.bottom
            topMargin: 5
            verticalCenter: parent.verticalCenter
            horizontalCenter: parent.horizontalCenter
        }
        icon: "kget"
    }
    Components.Label {
        id: errorLabel
        height: paintedHeight
        wrapMode: Text.WordWrap
        text: "KGet is not running."
        anchors.horizontalCenter: parent.horizontalCenter
    }
}