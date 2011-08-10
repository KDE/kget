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

#ifndef FILEDLG_H
#define FILEDLG_H

#include "../../core/basedialog.h"
#include "ui_filedlg.h"
#include "ui_urlwidget.h"
#include "ui_commondata.h"

namespace KGetMetalink
{
    class File;
}

class QSortFilterProxyModel;
class VerificationModel;
class UrlWidget;

class FileDlg : public KGetSaveSizeDialog
{
    Q_OBJECT

    public:
        FileDlg(KGetMetalink::File *file, const QStringList &currentFileNames, QSortFilterProxyModel *countrySort, QSortFilterProxyModel *languageSort, QWidget *parent, bool edit = false);

    Q_SIGNALS:
        void addFile();
        void fileEdited(const QString &oldFileName, const QString &newFileName);

    private slots:
        void slotUpdateOkButton();
        void slotOkClicked();

        //verification stuff
        void slotUpdateVerificationButtons();
        void slotAddHash();
        void slotRemoveHash();

    private:
        KGetMetalink::File *m_file;
        QString m_initialFileName;
        QStringList m_currentFileNames;
        VerificationModel *m_verificationModel;
        QSortFilterProxyModel *m_verificationProxy;
        bool m_edit;
        UrlWidget *m_urlWidget;
        Ui::FileDlg ui;
        Ui::CommonData uiData;
        QHash<QString, int> m_diggestLength;
};

#endif
