/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include <kurl.h>
#include <kdebug.h>

#include "core/transfer.h"
#include "core/transfergroup.h"
#include "core/transfergrouphandler.h"

TransferGroup::TransferGroup(Scheduler * scheduler, const QString & name)
    : JobQueue(scheduler),
      m_handler(0), m_name(name),
      m_totalSize(0), m_processedSize(0),
      m_percent(0), m_speed(0)
{

}

TransferGroup::~TransferGroup()
{
    handler()->postDeleteEvent();
}

void TransferGroup::append(Transfer * transfer)
{
    kdDebug() << "TransferGroup::append" << endl;
    JobQueue::append(transfer);

    handler()->postAddedTransferEvent(transfer);
}

void TransferGroup::prepend(Transfer * transfer)
{
    JobQueue::prepend(transfer);

    handler()->postAddedTransferEvent(transfer);
}

void TransferGroup::remove(Transfer * transfer)
{
    JobQueue::remove(transfer);

    handler()->postRemovedTransferEvent(transfer);
}

void TransferGroup::move(Transfer * transfer, int position)
{
    JobQueue::move(transfer, position);

    handler()->postMovedTransferEvent(transfer, position);
}

int TransferGroup::size() const
{
    return JobQueue::size();
}

Transfer * TransferGroup::findTransfer(KURL src)
{
    iterator it = begin();
    iterator itEnd = end();

    for(; it!=itEnd ; ++it)
    {
        Transfer * t = (Transfer *) *it;
        if( t->source().url() == src.url() )
            return t;
    }
    return 0;
}

TransferGroupHandler * TransferGroup::handler()
{
    if(!m_handler)
        m_handler = new TransferGroupHandler(this, scheduler());
    return m_handler;
}
