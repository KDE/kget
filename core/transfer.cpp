/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/


#include <qdom.h>

#include "transfer.h"
#include "transferhandler.h"
#include "scheduler.h"

Transfer::Transfer(TransferGroup * parent, Scheduler * scheduler,
                   const KURL & source, const KURL & dest)
    : Job(scheduler), m_group(parent),
      m_source(source), m_dest(dest)
{
    m_handler = new TransferHandler(this, scheduler);
}

Transfer::Transfer(TransferGroup * parent, Scheduler * scheduler, QDomNode * n)
    : Job(scheduler),
      m_group(parent)
{
    read(n);
    m_handler = new TransferHandler(this, scheduler);
}

const KURL & Transfer::source() const
{
    return m_source;
}

const KURL & Transfer::dest() const
{
    return m_dest;
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
    m_handler->setTransferChange(change, postEvent);
}
