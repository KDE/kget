/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#ifndef _TRANSFERDETAILS_H
#define _TRANSFERDETAILS_H

#include <qwidget.h>

#include "core/observer.h"
#include "core/transferhandler.h"

class QLabel;
class QProgressBar;

class TransferDetails : public QWidget, public TransferObserver
{
    Q_OBJECT
    public:
        TransferDetails(TransferHandler * transfer);

        void transferChangedEvent(TransferHandler * transfer);

    private:
        TransferHandler * m_transfer;
        QWidget         * m_detailsWidget;

        QLabel          * m_statusPixmapLabel;
        QLabel          * m_statusTextLabel;
        QLabel          * m_completedLabel;
        QLabel          * m_speedLabel;
        QProgressBar    * m_progressBar;
};

#endif
