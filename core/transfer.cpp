/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>

#include <QDomElement>
//Added by qt3to4:
#include <QPixmap>

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
    else
    {
        setStatus(status(), i18n("Stopped"), SmallIcon("stop"));
    }
}

Transfer::~Transfer()
{
    if(status() == Job::Delayed)
        m_scheduler->stopDelayTimer(this);

    delete(m_handler);
}

void Transfer::setDelay(int seconds)
{
    m_scheduler->startDelayTimer(this, seconds);

    setStatus(Job::Delayed, i18n("Delayed"), SmallIcon("tool_timer"));

    setTransferChange(Tc_Status, true);
}

void Transfer::delayTimerEvent()
{
    setStatus(Job::Stopped, i18n("Stopped"), SmallIcon("stop"));

    setTransferChange(Tc_Status, true);
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
    e.setAttribute("TotalSize", (qulonglong) m_totalSize);
    e.setAttribute("ProcessedSize", (qulonglong) m_processedSize);
}

void Transfer::load(QDomElement e)
{
    m_source = KURL::fromPathOrURL(e.attribute("Source"));
    m_dest = KURL::fromPathOrURL(e.attribute("Dest"));
    m_totalSize = e.attribute("TotalSize").toInt();
    m_processedSize = e.attribute("ProcessedSize").toInt();

    if( m_totalSize != 0)
        m_percent = (int)((100.0 * m_processedSize) / m_totalSize);
    else
        m_percent = 0;

    if((m_totalSize == m_processedSize) && (m_totalSize != 0))
    {
        setStatus(Job::Finished, i18n("Finished"), SmallIcon("ok"));
    }
    else
    {
        setStatus(status(), i18n("Stopped"), SmallIcon("stop"));
    }
}

void Transfer::setStatus(Job::Status jobStatus, QString text, QPixmap pix)
{
    //If a job is finished don't let it to be changed
    if((status() == Job::Finished) && (jobStatus != Job::Finished))
        return;

    m_statusText = text;
    m_statusPixmap = pix;

    /**
    * It's important to call job::setStatus AFTER having changed the 
    * icon or the text or whatever.
    * This becouse this function also notifies about this change
    * the scheduler which could also decide to change it another time
    * as well. For example if a job status is set to Aborted, the scheduler
    * could mark it to Delayed. This could trigger another icon or text
    * change which would be the right one since the status of the Job
    * has changed. If we set the icon or text after calling setStatus(),
    * we can overwrite the last icon or text change.
    */
    Job::setStatus(jobStatus);
}

void Transfer::setTransferChange(ChangesFlags change, bool postEvent)
{
    handler()->setTransferChange(change, postEvent);
}
