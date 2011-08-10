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

#ifndef MIRRORSETTINGS_H
#define MIRRORSETTINGS_H

#include "mirrormodel.h"
#include "../../core/basedialog.h"

#include "ui_mirrorsettings.h"
#include "ui_mirroradddlg.h"

class QSortFilterProxyModel;
class TransferHandler;

class MirrorAddDlg : public KDialog
{
    Q_OBJECT

    public:
        explicit MirrorAddDlg(MirrorModel *model, QWidget *parent = 0, Qt::WFlags flags = 0);
        explicit MirrorAddDlg(MirrorModel *model, QSortFilterProxyModel *countryModel, QWidget *parent = 0, Qt::WFlags flags = 0);
        
        virtual QSize sizeHint() const;

        /**
         * Shows or hides elements, by default all (expect MirrorItem::Used) are shown
         * @param type the type whose visibility should be modified
         * @param show if type should be shown or not
         * @note MirrorItem::Used and MirrorItem::Url can not be modified
         */
        void showItem(MirrorItem::DataType type, bool show);

    private slots:
        void addMirror();
        void updateButton(const QString &text = QString());

    private:
        void init();

    private:
        Ui::MirrorAddDlg ui;
        MirrorModel *m_model;
        QSortFilterProxyModel *m_countryModel;
};

class MirrorSettings : public KGetSaveSizeDialog
{
    Q_OBJECT

    public:
        MirrorSettings(QWidget *parent, TransferHandler *handler, const KUrl &file);

        virtual QSize sizeHint() const;

    private slots:
        void updateButton();
        void addClicked();
        void removeMirror();
        void save();

    private:
        TransferHandler *m_transfer;
        KUrl m_file;
        MirrorModel *m_model;
        MirrorProxyModel *m_proxy;
        Ui::MirrorSettings ui;
};

#endif
