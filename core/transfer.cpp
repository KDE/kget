/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2008 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "core/transfer.h"

#include "core/transferhandler.h"
#include "core/plugin/transferfactory.h"
#include "core/scheduler.h"

#include <kiconloader.h>
#include <klocale.h>

#include <QDomElement>
#include <QTime>

#ifdef HAVE_NEPOMUK
#include "nepomukhandler.h"
#endif

Transfer::Transfer(TransferGroup * parent, TransferFactory * factory,
                   Scheduler * scheduler, const KUrl & source, const KUrl & dest,
                   const QDomElement * e)
    : Job(parent, scheduler),
      m_source(source), m_dest(dest),
      m_totalSize(0), m_downloadedSize(0), m_uploadedSize(0),
      m_percent(0), m_downloadSpeed(0), m_uploadSpeed(0),
      m_uploadLimit(0), m_downloadLimit(0), m_isSelected(false),
      m_visibleUploadLimit(0), m_visibleDownloadLimit(0), m_ratio(0),
      m_handler(0), m_factory(factory)
{
    if( e )
        load( *e );
    else
    {
        setStatus(status(), i18nc("transfer state: stopped", "Stopped"), SmallIcon("process-stop"));
    }

#ifdef HAVE_NEPOMUK
    m_nepomukHandler = new NepomukHandler(this, 0);
#endif
}

Transfer::~Transfer()
{
    if(status() == Job::Delayed)
        m_scheduler->stopDelayTimer(this);

    delete(m_handler);
}

int Transfer::elapsedTime() const
{
    if (status() == Job::Running)
        return m_runningTime.elapsed() / 1000;

    return m_runningSeconds;
}

void Transfer::setUploadLimit(int ulLimit, SpeedLimit limit)
{
    if (limit == Transfer::VisibleSpeedLimit)
        m_visibleUploadLimit = ulLimit;
        if (ulLimit < m_uploadLimit || m_uploadLimit == 0)
            m_uploadLimit = ulLimit;
    else
        m_uploadLimit = ulLimit;

    setSpeedLimits(m_uploadLimit, m_downloadLimit);
}

void Transfer::setDownloadLimit(int dlLimit, SpeedLimit limit)
{
    if (limit == Transfer::VisibleSpeedLimit)
        m_visibleDownloadLimit = dlLimit;
        if (dlLimit < m_downloadLimit || m_downloadLimit == 0)
            m_downloadLimit = dlLimit;
    else
        m_downloadLimit = dlLimit;

    setSpeedLimits(m_uploadLimit, m_downloadLimit);
}

int Transfer::uploadLimit(SpeedLimit limit) const
{
    if (limit == Transfer::VisibleSpeedLimit)
        return m_visibleUploadLimit;

    return m_uploadLimit;
}

int Transfer::downloadLimit(SpeedLimit limit) const
{
    if (limit == Transfer::VisibleSpeedLimit)
        return m_visibleDownloadLimit;

    return m_downloadLimit;
}

void Transfer::setMaximumShareRatio(double ratio)
{
    m_ratio = ratio;
    checkShareRatio();
}

void Transfer::checkShareRatio()
{
    if (m_downloadedSize == 0 || m_ratio == 0)
        return;

    if (m_uploadedSize / m_downloadedSize >= m_ratio)
        setDownloadLimit(1, Transfer::InvisibleSpeedLimit);//If we set it to 0 we would have no limit xD
    else
        setDownloadLimit(0, Transfer::InvisibleSpeedLimit);
}

void Transfer::setDelay(int seconds)
{
    m_scheduler->startDelayTimer(this, seconds);

    setStatus(Job::Delayed, i18nc("transfer state: delayed", "Delayed"), SmallIcon("view-history"));

    setTransferChange(Tc_Status, true);
}

void Transfer::delayTimerEvent()
{
    setStatus(Job::Stopped, i18nc("transfer state: stopped", "Stopped"), SmallIcon("process-stop"));

    setTransferChange(Tc_Status, true);
}

void Transfer::setLog(const QString& message, LogLevel level)
{
    QString msg("<font color=\"blue\">" + QTime::currentTime().toString() + "</font> : ");
    if (level == error)
    {
        msg += "<font color=\"red\">" + message + "</font>";
    }
    if (level == warning)
    {
        msg += "<font color=\"yellow\">" + message + "</font>";
    } else {
        msg += message;
    }
    m_log << msg;
}

TransferHandler * Transfer::handler()
{
    if(!m_handler)
        m_handler = m_factory->createTransferHandler(this, scheduler());

    return m_handler;
}

TransferTreeModel * Transfer::model()
{
    return group()->model();
}

void Transfer::save(const QDomElement &element)
{
    QDomElement e = element;
    e.setAttribute("Source", m_source.url());
    e.setAttribute("Dest", m_dest.url());
    e.setAttribute("TotalSize", m_totalSize);
    e.setAttribute("DownloadedSize", m_downloadedSize);
    e.setAttribute("UploadedSize", m_uploadedSize);
    e.setAttribute("DownloadLimit", m_visibleDownloadLimit);
    e.setAttribute("UploadLimit", m_visibleUploadLimit);
    e.setAttribute("ElapsedTime", status() == Job::Running ? m_runningTime.elapsed() / 1000 : m_runningSeconds);
}

void Transfer::load(const QDomElement &e)
{
    m_source = KUrl(e.attribute("Source"));
    m_dest = KUrl(e.attribute("Dest"));

    m_totalSize = e.attribute("TotalSize").toULongLong();
    m_downloadedSize = e.attribute("DownloadedSize").toULongLong();
    m_uploadedSize = e.attribute("UploadedSize").toULongLong();

    if( m_totalSize != 0)
        m_percent = (int)((100.0 * m_downloadedSize) / m_totalSize);
    else
        m_percent = 0;

    if((m_totalSize == m_downloadedSize) && (m_totalSize != 0))
    {
        setStatus(Job::Finished, i18nc("transfer state: finished", "Finished"), SmallIcon("dialog-ok"));
    }
    else
    {
        setStatus(status(), i18nc("transfer state: stopped", "Stopped"), SmallIcon("process-stop"));
    }
    setUploadLimit(e.attribute("UploadLimit").toInt(), Transfer::VisibleSpeedLimit);
    setDownloadLimit(e.attribute("DownloadLimit").toInt(), Transfer::VisibleSpeedLimit);
    m_runningSeconds = e.attribute("ElapsedTime").toInt();
}

void Transfer::setStatus(Job::Status jobStatus, const QString &text, const QPixmap &pix)
{
    //If a job is finished don't let it to be changed
    if((status() == Job::Finished) && (jobStatus != Job::Finished))
        return;

    m_statusText = text;
    m_statusPixmap = pix;

#ifdef HAVE_NEPOMUK
    if (jobStatus == Job::Finished)
        m_nepomukHandler->saveFileProperties();
#endif

    if (jobStatus == Job::Running && status() != Job::Running)
    {
        m_runningTime.restart();
        m_runningTime.addSecs(m_runningSeconds);
    }
    if (jobStatus != Job::Running && status() == Job::Running)
        m_runningSeconds = m_runningTime.elapsed() / 1000;
    /**
    * It's important to call job::setStatus AFTER having changed the 
    * icon or the text or whatever.
    * This because this function also notifies about this change
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
