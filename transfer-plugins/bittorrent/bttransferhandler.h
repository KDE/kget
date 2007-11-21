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

class Scheduler;

class BTTransferHandler : public TransferHandler
{
    public:
        BTTransferHandler(BTTransfer * transfer, Scheduler * scheduler);

        int chunksTotal()       {return m_transfer->chunksTotal();}
        int chunksDownloaded()  {return m_transfer->chunksDownloaded();}
        int peersConnected()    {return m_transfer->peersConnected();}
        int peersNotConnected() {return m_transfer->peersNotConnected();}

    private:
        BTTransfer * m_transfer;
};

#endif
