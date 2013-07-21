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

Item {

    id: container
    height: 100
    width: 200

    property int minimumWidth: 200
    property int minimumHeight: 100

    PlasmaCore.DataSource {
        id: kgetSource
        engine: "kget"
        interval: 500
        onSourceAdded: {
            if (source=="Error") {
                connectSource(source)
                state="error"
            } else {
                connectSource(source)
            }
        }
        onSourceRemoved: {
            if (source=="Error") {
                disconnectSource(source)
                setup()
                state=""
            } else {
                disconnectSource(source)
            }
        }
    }
    function setup() {
        var sources = kgetSource.sources;
        for(i=0;i<sources.length;i++) {
            kgetSource.connectSource(sources[i]);
        }
    }

    Component.onCompleted: setup()

    MessageItem {
        id: msgBox
        anchors.fill: parent
        visible: false
    }

    ScrollableListView {
        id: downloadList
        anchors.fill: parent
        visible: true
        model: PlasmaCore.DataModel {
            dataSource: kgetSource
        }
        delegate: DownloadItem {}
    }

    states:[
    State {
        name: ""
        PropertyChanges{ target: downloadList; visible: true }
        PropertyChanges { target: msgBox; visible: false }
    },
    State {
        name: "error"
        PropertyChanges { target: downloadList; visible: false }
        PropertyChanges { target: msgBox; visible: true }
    }
    ]
}
