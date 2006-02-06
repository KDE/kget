/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#ifndef BTDETAILSWIDGET_H
#define BTDETAILSWIDGET_H

#include <QWidget>

#include "core/observer.h"

class QLabel;

class BTTransferHandler;

class BTDetailsWidget : public QWidget, public TransferObserver
{
    Q_OBJECT
    public:
        BTDetailsWidget(BTTransferHandler * transfer);

        void transferChangedEvent(TransferHandler * transfer);

    private:
        BTTransferHandler * m_transfer;

        QLabel * m_chunksTotalLabel;
        QLabel * m_chunksDownloadedLabel;
        QLabel * m_peersConnectedLabel;
        QLabel * m_peersNotConnectedLabel;
};

#endif
