/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef TRANSFERDETAILS_H
#define TRANSFERDETAILS_H

#include <QWidget>

#include "core/plugin/transferfactory.h"
#include "core/transferhandler.h"
#include "ui_transferdetailsfrm.h"

class QVBoxLayout;

class TransferDetails : public QWidget
{
    Q_OBJECT
public:
    TransferDetails(TransferHandler *transfer);
    ~TransferDetails() override;

    // gets the generic details widget if the transfer factory doesn't override it
    static QWidget *detailsWidget(TransferHandler *transfer);

public Q_SLOTS:
    void slotTransferChanged(TransferHandler *transfer, TransferHandler::ChangesFlags flags);

private:
    TransferHandler *m_transfer = nullptr;
    QVBoxLayout *m_layout = nullptr;
    QWidget *m_genericWidget = nullptr;

    Ui::TransferDetailsFrm frm;
};

#endif
