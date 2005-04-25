/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include <qdom.h>

#include <kurl.h>
#include <kdebug.h>

#include "core/transfer.h"
#include "core/transfergroup.h"
#include "core/transfergrouphandler.h"
#include "core/model.h"

TransferGroup::TransferGroup(Scheduler * scheduler, const QString & name)
    : JobQueue(scheduler),
      m_handler(0), m_name(name),
      m_totalSize(0), m_processedSize(0),
      m_percent(0), m_speed(0)
{

}

TransferGroup::TransferGroup(Scheduler * scheduler, const QDomElement & e)
    : JobQueue(scheduler),
      m_handler(0),
      m_totalSize(0), m_processedSize(0),
      m_percent(0), m_speed(0)
{
    load( e );
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

void TransferGroup::save(QDomElement e)
{
    e.setAttribute("Name", m_name);

    iterator it = begin();
    iterator itEnd = end();

    for( ; it!=itEnd; ++it )
    {
        QDomElement t = e.ownerDocument().createElement("Transfer");
        e.appendChild(t);
        ((Transfer *) *it)->save(t);
    }
}

void TransferGroup::load(const QDomElement & e)
{
    kdDebug() << "TransferGroup::load" << endl;

    m_name = e.attribute("Name");

    QDomNodeList nodeList = e.elementsByTagName("Transfer");
    int nItems = nodeList.length();

    for(int i=0; i<nItems; i++)
    {
        kdDebug() << "TransferGroup::load -> addTransfer" << endl;
        Model::addTransfer( nodeList.item(i).toElement(), name() );
    }
}
