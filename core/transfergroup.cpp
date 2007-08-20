/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include "core/transfergroup.h"

#include "core/transfer.h"
#include "core/transfergrouphandler.h"
#include "core/kget.h"

#include <kdebug.h>

#include <QDomElement>


TransferGroup::TransferGroup(TransferTreeModel * model, Scheduler * scheduler, const QString & name)
    : JobQueue(scheduler),
      m_model(model), m_handler(0), m_name(name),
      m_totalSize(0), m_processedSize(0),
      m_percent(0), m_speed(0)
{

}

TransferGroup::~TransferGroup()
{
    handler()->postDeleteEvent();
}

void TransferGroup::setStatus(Status queueStatus)
{
    JobQueue::setStatus(queueStatus);

    handler()->postGroupChangedEvent();
}

void TransferGroup::append(Transfer * transfer)
{
    kDebug(5001) << "TransferGroup::append";

    Transfer * after;
    if(size() == 0) 
        after = 0;
    else
        after = static_cast<Transfer *> (last());

    JobQueue::append(transfer);

    handler()->postAddedTransferEvent(transfer, after);
}

void TransferGroup::prepend(Transfer * transfer)
{
    JobQueue::prepend(transfer);

    handler()->postAddedTransferEvent(transfer, 0);
}

void TransferGroup::insert(Transfer * transfer, Transfer * after)
{
    JobQueue::insert(transfer, after);

    handler()->postAddedTransferEvent(transfer, after);
}

void TransferGroup::remove(Transfer * transfer)
{
    JobQueue::remove(transfer);

    handler()->postRemovedTransferEvent(transfer);
}

void TransferGroup::move(Transfer * transfer, Transfer * after)
{
    if(transfer == after)
        return;

    TransferGroup * oldTransferGroup = transfer->group();

    JobQueue::move(transfer, after);

    if(oldTransferGroup == this)
        handler()->postMovedTransferEvent(transfer, after);
    else
    {
        handler()->postAddedTransferEvent(transfer, after);
        oldTransferGroup->handler()->postRemovedTransferEvent(transfer);
    }
}

Transfer * TransferGroup::findTransfer(const KUrl &src)
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

Transfer * TransferGroup::operator[] (int i) const
{
//     kDebug(5001) << "TransferGroup::operator[]";

    return (Transfer *)((* (JobQueue *)this)[i]);
}

TransferGroupHandler * TransferGroup::handler()
{
    if(!m_handler)
    {
        m_handler = new TransferGroupHandler(this, scheduler());
    }
    return m_handler;
}

void TransferGroup::transferChangedEvent(Transfer * transfer)
{
    m_handler->postGroupChangedEvent();
}

void TransferGroup::save(QDomElement e) // krazy:exclude=passbyvalue
{
    kDebug(5001) << "TransferGroup::save()  -->  " << name();

    e.setAttribute("Name", m_name);

    iterator it = begin();
    iterator itEnd = end();

    for( ; it!=itEnd; ++it )
    {
        kDebug(5001) << "TransferGroup::save()  -->" << name() << "  transfer: " << ((Transfer *) *it)->source();
        QDomElement t = e.ownerDocument().createElement("Transfer");
        e.appendChild(t);
        ((Transfer *) *it)->save(t);
    }
}

void TransferGroup::load(const QDomElement & e)
{
    kDebug(5001) << "TransferGroup::load";

    m_name = e.attribute("Name");

    QDomNodeList nodeList = e.elementsByTagName("Transfer");
    int nItems = nodeList.length();

    for(int i=0; i<nItems; i++)
    {
        kDebug(5001) << "TransferGroup::load -> addTransfer";
        KGet::addTransfer( nodeList.item(i).toElement(), name() );
    }
}
