/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2007 by Javier Goday <jgoday@gmail.com>
   Copyright (C) 2008 by Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef NEW_TRANSFER_DIALOG_H
#define NEW_TRANSFER_DIALOG_H

#include <KDialog>
#include <KUrl>

#include "ui_newtransferwidget.h"

/**
* Dialog to allow add one or more transfers to kget.
* If only one transfer is added then the dialog shows a KUrlRequester.
* If a list of transfers are added then the dialog shows a KListWidget (multiple = true)
* with the transfers as checkable items.
* Also display a KUrlComboRequester for the destination file (or folder if multiple = true)
* And a QComboBox with the groups of transfer in case there are more than one
* 
* This class is a singleton, only one instance is allowed.
* If a source is added and the dialog is already showed, 
* the dialog becomes multiple and shows the KListWidget
* adding the new transfer to the list of previous ones.
*/
class NewTransferDialog : public KDialog, Ui::NewTransferWidget
{
    Q_OBJECT
public:
    NewTransferDialog(QWidget *parent = 0);
    ~NewTransferDialog();

    /**
    * Returns the current instance of the 'new transfer' dialog
    */
    static NewTransferDialog *instance();

    /**
    * Shows the dialog adding one transfer url.
    * If the dialog is already displayed, 
    * then add the url to the others or other transfer, 
    * and displayed the sources as a listWidget (multiple = true)
    */
    void showDialog(const QString &srcUrl = QString());

    /**
    * Shows the dialog adding one url list transfers
    */
    void showDialog(const KUrl::List &list);

public slots:
    /**
    * Called when the transfer group or the urlREquester changed, the dialog sets the default destination 
    * for transfers in the new group
    */
    void setDefaultDestination();

private:
    void prepareDialog();
    void resizeDialog();

    class Private;
    NewTransferDialog::Private *d;

    KUrl::List m_sources;
};

#endif
