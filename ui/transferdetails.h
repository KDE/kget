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

#include "core/transferhandler.h"
#include "core/plugin/transferfactory.h"
#include "ui_transferdetailsfrm.h"

class QLabel;
class QProgressBar;
class QVBoxLayout;

class TransferDetails : public QWidget
{
    Q_OBJECT
    public:
        TransferDetails(TransferHandler * transfer);
        ~TransferDetails();

        // gets the generic details widget if the transfer factory doesn't override it
        static QWidget *detailsWidget(TransferHandler *transfer);
        
    public slots:
        void slotTransferChanged(TransferHandler * transfer, TransferHandler::ChangesFlags flags);
        
    private:
        TransferHandler * m_transfer;
        QVBoxLayout     * m_layout;
        QWidget         * m_genericWidget;

        Ui::TransferDetailsFrm frm;
};

#endif
