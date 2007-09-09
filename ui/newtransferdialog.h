/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2007 by Javier Goday <jgoday@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef NEW_TRANSFER_DIALOG_H
#define NEW_TRANSFER_DIALOG_H

#include <QWidget>
#include <KDialog>

#include "ui_newtransferdialog.h"

class NewTransferWidget : public QWidget, Ui::NewTransferWidget
{
    Q_OBJECT

public:
    NewTransferWidget(QWidget *parent=0);

    void setFolderPath(QString folder);
    void setUrl(QString url);

    QString folderPath() const;
    QString url() const;
    QString groupName() const;
    bool setAsDefaultFolder() const;
};

class NewTransferDialog : public KDialog
{
    Q_OBJECT

public:
    NewTransferDialog(QWidget *parent=0);
    ~NewTransferDialog();

    static void showNewTransferDialog();
    NewTransferWidget *transferWidget();

protected slots:
    void slotButtonClicked(int button);

private:
    NewTransferWidget *m_transferWidget;
};

#endif
