/* This file is part of the KDE project

   Copyright (C) 2008 Lukas Appelhans <l.appelhans@gmx.de>
   Copyright (C) 2009 Matthias Fuchs <mat69@gmx.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef TRANSFERSETTINGSDIALOG_H
#define TRANSFERSETTINGSDIALOG_H

#include "ui_transfersettingsdialog.h"

#include <KDialog>

class FileModel;
class QSortFilterProxyModel;
class RenameFile;
class TransferHandler;

class TransferSettingsDialog : public KDialog
{
    Q_OBJECT
    public:
        TransferSettingsDialog(QWidget *parent, TransferHandler *transfer);
        ~TransferSettingsDialog();

    private slots:
        void slotMirrors();
        void slotRename();
        void slotVerification();
        void slotSelectionChanged();
        void slotFinished();
        void save();

    private:
        TransferHandler *m_transfer;
        FileModel *m_model;
        QSortFilterProxyModel *m_proxy;
        Ui::TransferSettingsDialog ui;
};

#endif

