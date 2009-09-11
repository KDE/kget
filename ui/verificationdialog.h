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

#include <KDialog>

class FileModel;
class TransferHandler;
class Verifier;
class VerificationModel;

class VerificationAddDlg : public KDialog
{
    Q_OBJECT

    public:
        explicit VerificationAddDlg(VerificationModel *model, QWidget *parent = 0, Qt::WFlags flags = 0);

    private slots:
        void addChecksum();

        /**
         * Adds a checksum and prepares the dialog to add more items
         */
        void addMore();

        void updateButton();

    private:
        Ui::VerificationAddDlg ui;
        VerificationModel *m_model;
        QHash<QString, int> m_diggestLength;
};

class VerificationDialog : public KDialog
{
    Q_OBJECT

    public:
        VerificationDialog(QWidget *parent, TransferHandler *transfer, const KUrl &file);

    private slots:
        void updateButtons();
        void addPressed();
        void removePressed();
        void verifyPressed();
        void slotVerified(bool verified);

    private:
        TransferHandler *m_transfer;
        KUrl m_file;
        Verifier *m_verifier;
        VerificationModel *m_model;
        FileModel *m_fileModel;
        Ui::VerificationDialog ui;
};

#endif //VERIFICATIONDIALOG_H
