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

#ifndef DRAGDLG_H
#define DRAGDLG_H

#include "../../core/basedialog.h"
#include "ui_dragdlg.h"
#include "ui_commondata.h"

class QCheckBox;
class QSortFilterProxyModel;
class UrlWidget;

namespace KGetMetalink
{
    class CommonData;
    class Resources;
}

class DragDlg : public KGetSaveSizeDialog
{
    Q_OBJECT

    public:
        DragDlg(KGetMetalink::Resources *resources, KGetMetalink::CommonData *commonData, QSortFilterProxyModel *countrySort, QSortFilterProxyModel *languageSort, QWidget *parent = 0);

    signals:
        /**
         * The types the user want to be checked and if partial checksums should be created
         * @note emitted whenever the dialog is closed, so might be empty
         */
        void usedTypes(const QStringList &types, bool createPartial);

    private slots:
        /**
         * Called when the dialog is finished, to emit usedTypes
         */
        void slotFinished();

    private:
        UrlWidget *m_urlWidget;
        KGetMetalink::Resources *m_resources;
        KGetMetalink::CommonData *m_commonData;
        QList<QCheckBox*> m_checkBoxes;
        Ui::DragDlg ui;
        Ui::CommonData uiData;
};

#endif
