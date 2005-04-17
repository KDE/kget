/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include <kdebug.h>

#include <qdom.h>

#include "core/transfer.h"
#include "core/transferhandler.h"
#include "core/transfergroup.h"
#include "core/scheduler.h"

Transfer::Transfer(TransferGroup * parent, TransferFactory * factory,
                   Scheduler * scheduler, const KURL & source, const KURL & dest)
    : Job(parent, scheduler),
      m_source(source), m_dest(dest),
      m_handler(0), m_factory(factory),
      m_totalSize(0), m_processedSize(0),
      m_percent(0), m_speed(0)
{

}

Transfer::Transfer(TransferGroup * parent, TransferFactory * factory, 
                   Scheduler * scheduler, QDomNode * n)
    : Job(parent, scheduler)
{
    read(n);
}

const KURL & Transfer::source() const
{
    return m_source;
}

const KURL & Transfer::dest() const
{
    return m_dest;
}

TransferHandler * Transfer::handler()
{
    if(!m_handler)
        m_handler = new TransferHandler(this, scheduler());

    return m_handler;
}

void Transfer::read(QDomNode * n)
{
    QDomElement e = n->toElement();

    m_totalSize = e.attribute("TotalSize").toInt();
    m_processedSize = e.attribute("ProcessedSize").toInt();
    m_percent = e.attribute("Percent").toULong();
}

void Transfer::write(QDomNode * n)
{
    QDomElement t = n->ownerDocument().createElement("Transfer");
    n->appendChild(t);

    t.setAttribute("TotalSize", m_totalSize);
    t.setAttribute("ProcessedSize", m_processedSize);
    t.setAttribute("Percent", m_percent);
}

void Transfer::setTransferChange(ChangesFlags change, bool postEvent)
{
    handler()->setTransferChange(change, postEvent);
}
