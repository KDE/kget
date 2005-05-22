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
                   Scheduler * scheduler, const KURL & source, const KURL & dest,
                   const QDomElement * e)
    : Job(parent, scheduler),
      m_source(source), m_dest(dest),
      m_totalSize(0), m_processedSize(0),
      m_percent(0), m_speed(0),
      m_isSelected(false),
      m_handler(0), m_factory(factory)
{
    if( e )
        load( *e );
}

Transfer::~Transfer()
{
    delete(m_handler);
}

TransferHandler * Transfer::handler()
{
    if(!m_handler)
        m_handler = new TransferHandler(this, scheduler());

    return m_handler;
}

void Transfer::save(QDomElement e)
{
    e.setAttribute("Source", m_source.url());
    e.setAttribute("Dest", m_dest.url());
    e.setAttribute("TotalSize", m_totalSize);
    e.setAttribute("ProcessedSize", m_processedSize);
}

void Transfer::load(QDomElement e)
{
    m_source = KURL::fromPathOrURL(e.attribute("Source"));
    m_dest = KURL::fromPathOrURL(e.attribute("Dest"));
    m_totalSize = e.attribute("TotalSize").toInt();
    m_processedSize = e.attribute("ProcessedSize").toInt();

    if(m_totalSize != 0)
        m_percent = (int)((100.0 * m_processedSize) / m_totalSize);
    else
        m_percent = 0;
}

void Transfer::setTransferChange(ChangesFlags change, bool postEvent)
{
    handler()->setTransferChange(change, postEvent);
}
