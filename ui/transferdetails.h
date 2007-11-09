/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef _TRANSFERDETAILS_H
#define _TRANSFERDETAILS_H

#include <QWidget>

#include "core/observer.h"
#include "core/transferhandler.h"
#include "core/plugin/transferfactory.h"

class QLabel;
class QProgressBar;
class QVBoxLayout;

class TransferDetails : public QWidget, public TransferObserver
{
    Q_OBJECT
    public:
        TransferDetails(TransferHandler * transfer);
        ~TransferDetails();

        void transferChangedEvent(TransferHandler * transfer);

    private:
        TransferHandler * m_transfer;
        QVBoxLayout     * m_layout;
        QWidget         * m_genericWidget;
        QWidget         * m_detailsWidget;

        QLabel          * m_statusPixmapLabel;
        QLabel          * m_statusTextLabel;
        QLabel          * m_completedLabel;
        QLabel          * m_speedLabel;
        QProgressBar    * m_progressBar;
};

#endif
