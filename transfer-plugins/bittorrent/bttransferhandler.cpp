/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include "bttransfer.h"
#include "bttransferhandler.h"

#include "core/scheduler.h"

BTTransferHandler::BTTransferHandler(BTTransfer * transfer, Scheduler * scheduler)
    : TransferHandler(transfer, scheduler),
      m_transfer(transfer)
{

}


