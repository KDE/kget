/* This file is part of the KDE project

   Copyright (C) 2007 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "bttransferhandler.h"

#include "bttransfer.h"

#include "core/scheduler.h"

BTTransferHandler::BTTransferHandler(BTTransfer * transfer, Scheduler * scheduler)
    : TransferHandler(transfer, scheduler),
      m_transfer(transfer)
{

}


