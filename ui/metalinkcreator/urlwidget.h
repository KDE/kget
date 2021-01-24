/***************************************************************************
*   Copyright (C) 2009 Matthias Fuchs <mat69@gmx.net>                     *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
***************************************************************************/

#ifndef URLWIDGET_H
#define URLWIDGET_H

#include <QPointer>

#include "ui_urlwidget.h"

namespace KGetMetalink
{
    class Resources;
}

class MirrorModel;
class MirrorProxyModel;
class QSortFilterProxyModel;

class UrlWidget : public QObject
{
    Q_OBJECT

    public:
        UrlWidget(QObject *parent = nullptr);
        ~UrlWidget() override;

        void init(KGetMetalink::Resources *resources, QSortFilterProxyModel *countrySort);

        QWidget *widget();

        void save();

        bool hasUrls() const;

    signals:
        void urlsChanged();

    private slots:
        void slotUrlClicked();
        void slotAddMirror();
        void slotRemoveMirror();

    private:
        KGetMetalink::Resources *m_resources = nullptr;
        MirrorModel *m_mirrorModel = nullptr;
        MirrorProxyModel *m_proxy = nullptr;
        QSortFilterProxyModel *m_countrySort = nullptr;
        QPointer<QWidget> m_widget;
        Ui::UrlWidget ui;
};

#endif
