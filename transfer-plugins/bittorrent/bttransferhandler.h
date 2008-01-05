/* This file is part of the KDE project

   Copyright (C) 2007 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef _BTTRANSFERHANDLER_H
#define _BTTRANSFERHANDLER_H

#include "bttransfer.h"
#include "core/transferhandler.h"
#include <torrent/torrentcontrol.h>

class Scheduler;

class BTAdvancedDetailsWidget;

class BTSpeedLimits;

class BTTransferHandler : public QObject, public TransferHandler
{
    Q_OBJECT
    public:
        BTTransferHandler(BTTransfer * transfer, Scheduler * scheduler);

        int chunksTotal()                               {return m_transfer->chunksTotal();}
        int chunksDownloaded()                          {return m_transfer->chunksDownloaded();}
        int chunksExcluded()                            {return m_transfer->chunksExcluded();}
        int chunksLeft()                                {return m_transfer->chunksLeft();}
        int seedsConnected()                            {return m_transfer->seedsConnected();}
        int seedsDisconnected()                         {return m_transfer->seedsDisconnected();}
        int leechesConnected()                          {return m_transfer->leechesConnected();}
        int leechesDisconnected()                       {return m_transfer->leechesDisconnected();}
        int ulRate()                                    {return m_transfer->ulRate();}
        int dlRate()                                    {return m_transfer->dlRate();}
        bt::TorrentControl * torrentControl()           {return m_transfer->torrentControl();}
        int ulLimit()                                   {return m_transfer->ulLimit();}
        int dlLimit()                                   {return m_transfer->dlLimit();}
        int percent()                                   {return m_transfer->percent();}
        bool ready()                                    {return m_transfer->ready();}
        float maxShareRatio()                           {return m_transfer->maxShareRatio();}

        void addTracker(QString url)                    {m_transfer->addTracker(url);}
        void setTrafficLimits(int ulLimit, int dlLimit) {m_transfer->setTrafficLimits(ulLimit, dlLimit);}
        void setMaxShareRatio(float ratio)              {m_transfer->setMaxShareRatio(ratio);}

    public slots:
        void createAdvancedDetails();
        void createSpeedLimits();

    private slots:
        void removeAdvancedDetails();
        void removeSpeedLimits();

    private:
        BTTransfer * m_transfer;
        BTAdvancedDetailsWidget *advancedDetails;
        BTSpeedLimits *speedLimits;
};

#endif
