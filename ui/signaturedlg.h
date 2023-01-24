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

#ifndef SIGNATUREDLG_H
#define SIGNATUREDLG_H

#include <QModelIndex>
#include "ui_signaturedlg.h"
#include "../core/basedialog.h"

class FileModel;
class Signature;
class TransferHandler;

class SignatureDlg : public KGetSaveSizeDialog
{
    Q_OBJECT

    public:
        SignatureDlg(TransferHandler *transfer, const QUrl &dest, QWidget *parent = nullptr, Qt::WindowFlags flags = {});

    private Q_SLOTS:
        void fileFinished(const QUrl &file);
        void verifyClicked();
        void updateData();
        void updateButtons();
        void textChanged();
        void loadSignatureClicked();

    private:
        void clearData();
        void handleWidgets(bool isAsciiSig);

    private:
        Ui::SignatureDlg ui;
        Signature *m_signature = nullptr;
        FileModel *m_fileModel = nullptr;
        QModelIndex m_file;
};

#endif
