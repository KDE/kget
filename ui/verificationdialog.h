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

#ifndef VERIFICATIONDIALOG_H
#define VERIFICATIONDIALOG_H

#include "ui_verificationdialog.h"
#include "ui_verificationadddlg.h"

#include "../core/basedialog.h"

class FileModel;
class QSortFilterProxyModel;
class TransferHandler;
class Verifier;
class VerificationModel;

class VerificationAddDlg : public KDialog
{
    Q_OBJECT

    public:
        explicit VerificationAddDlg(VerificationModel *model, QWidget *parent = 0, Qt::WFlags flags = 0);
        
        virtual QSize sizeHint() const;

    private slots:
        void addChecksum();
        void updateButton();

    private:
        Ui::VerificationAddDlg ui;
        VerificationModel *m_model;
        QHash<QString, int> m_diggestLength;
};

class VerificationDialog : public KGetSaveSizeDialog
{
    Q_OBJECT

    public:
        VerificationDialog(QWidget *parent, TransferHandler *transfer, const KUrl &file);
        
        virtual QSize sizeHint() const;

    private slots:
        void fileFinished(const KUrl &file);
        void updateButtons();
        void addClicked();
        void removeClicked();
        void verifyClicked();
        void slotVerified(bool verified);
        void slotFinished();

    private:
        TransferHandler *m_transfer;
        Verifier *m_verifier;
        VerificationModel *m_model;
        QSortFilterProxyModel *m_proxy;
        FileModel *m_fileModel;
        QModelIndex m_file;
        Ui::VerificationDialog ui;
};

#endif //VERIFICATIONDIALOG_H
