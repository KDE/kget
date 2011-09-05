/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2010 Matthias Fuchs <mat69@gmx.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "core/transfergroup.h"

#include "core/transfergrouphandler.h"
#include "core/kget.h"

#include <KDebug>
#include <KMessageBox>
#include <KLocale>
#include <KStandardDirs>
#include <kio/global.h>

#include <QFile>
#include <QDateTime>
#include <QDomElement>


TransferGroup::TransferGroup(TransferTreeModel * model, Scheduler * parent, const QString & name)
    : JobQueue(parent),
      m_model(model), m_name(name),
      m_totalSize(0), m_downloadedSize(0), m_uploadedSize(0),
      m_percent(0), m_downloadSpeed(0), m_uploadSpeed(0),
      m_downloadLimit(0), m_uploadLimit(0),
      m_visibleDownloadLimit(0), m_visibleUploadLimit(0),
      m_iconName("bookmark-new-list"), m_defaultFolder()
{
    m_handler = new TransferGroupHandler(parent, this);
}

TransferGroup::~TransferGroup()
{
}

int TransferGroup::downloadSpeed()
{
    m_downloadSpeed = 0;
    foreach(Job *job, runningJobs())
    {
        Transfer *transfer = static_cast<Transfer*>(job);
        if (transfer)
            m_downloadSpeed += transfer->downloadSpeed();
    }
    return m_downloadSpeed;
}

int TransferGroup::uploadSpeed()
{
    m_uploadSpeed = 0;
    foreach(Job *job, runningJobs())
    {
        Transfer *transfer = static_cast<Transfer*>(job);
        if (transfer)
            m_uploadSpeed += transfer->uploadSpeed();
    }
    return m_uploadSpeed;
}

bool TransferGroup::supportsSpeedLimits()
{
    QList<Job*> jobs = runningJobs();
    foreach (Job *job, jobs) {
        Transfer * transfer = static_cast<Transfer*>(job);
        if (!(transfer->capabilities() & Transfer::Cap_SpeedLimit)) {
            return false;
        }
    }

    //empty jobs can't support a speed limit
    return !jobs.isEmpty();
}

void TransferGroup::setStatus(Status queueStatus)
{
    JobQueue::setStatus(queueStatus);

    m_handler->setGroupChange(Gc_Status, true);
}

void TransferGroup::append(Transfer * transfer)
{
    JobQueue::append(transfer);

    calculateSpeedLimits();
}

void TransferGroup::append(const QList<Transfer*> &transfers)
{
    QList<Job*> jobs;
    foreach (Transfer *transfer, transfers) {
        jobs << transfer;
    }
    JobQueue::append(jobs);

    calculateSpeedLimits();
}


void TransferGroup::prepend(Transfer * transfer)
{
    JobQueue::prepend(transfer);

    calculateSpeedLimits();
}

void TransferGroup::insert(Transfer * transfer, Transfer * after)
{
    JobQueue::insert(transfer, after);

    calculateSpeedLimits();
}

void TransferGroup::remove(Transfer * transfer)
{
    JobQueue::remove(transfer);

    calculateSpeedLimits();
}

void TransferGroup::remove(const QList<Transfer*> &transfers)
{
    QList<Job*> jobs;
    foreach (Transfer *transfer, transfers) {
        jobs << transfer;
    }
    JobQueue::remove(jobs);

    calculateSpeedLimits();
}

void TransferGroup::move(Transfer * transfer, Transfer * after)
{
    if(transfer == after)
        return;

    JobQueue::move(transfer, after);
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

Transfer *TransferGroup::findTransferByDestination(const KUrl &dest)
{
    iterator it = begin();
    iterator itEnd = end();

    for(; it!=itEnd ; ++it) {
        Transfer *t = (Transfer *) *it;
        if(t->dest().url() == dest.url()) {
            return t;
        }
    }
    return 0;
}

void TransferGroup::setUploadLimit(int ulLimit, Transfer::SpeedLimit limit) 
{
    if (limit == Transfer::VisibleSpeedLimit) {
        m_visibleUploadLimit = ulLimit;
        if (ulLimit < m_uploadLimit || m_uploadLimit == 0)
            m_uploadLimit = ulLimit;
    } else {
        m_uploadLimit = ulLimit;
    }

    calculateUploadLimit();
}

void TransferGroup::setDownloadLimit(int dlLimit, Transfer::SpeedLimit limit) 
{
    if (limit == Transfer::VisibleSpeedLimit) {
        m_visibleDownloadLimit = dlLimit;
        if (dlLimit < m_downloadLimit || m_downloadLimit == 0)
            m_downloadLimit = dlLimit;
    } else {
        m_downloadLimit = dlLimit;
    }

    calculateDownloadLimit();
}

int TransferGroup::uploadLimit(Transfer::SpeedLimit limit) const
{
    if (limit == Transfer::VisibleSpeedLimit)
        return m_visibleUploadLimit;

    return m_uploadLimit;
}

int TransferGroup::downloadLimit(Transfer::SpeedLimit limit) const
{
    if (limit == Transfer::VisibleSpeedLimit)
        return m_visibleDownloadLimit;

    return m_downloadLimit;
}

void TransferGroup::calculateSpeedLimits()
{
    kDebug(5001) << "We will calculate the new SpeedLimits now";
    calculateDownloadLimit();
    calculateUploadLimit();
}

void TransferGroup::calculateDownloadLimit()
{
    kDebug(5001) << "Calculate new DownloadLimit of " + QString::number(m_downloadLimit);
    if (supportsSpeedLimits())
    {
        const QList<Job*> running = runningJobs();
        int n = running.count();
        int pool = 0;//We create a pool where we have some KiB/s to go to other transfer's...
        QList<Transfer*> transfersNeedSpeed;
        foreach (Job *job, running) {
            Transfer * transfer = static_cast<Transfer*>(job);
            if (transfer)
            {
                if (m_downloadLimit == 0 && transfer->downloadLimit(Transfer::VisibleSpeedLimit) != 0)
                    continue;
                else if (m_downloadLimit == 0 && transfer->downloadLimit(Transfer::VisibleSpeedLimit) == 0)
                    transfer->setDownloadLimit(0, Transfer::InvisibleSpeedLimit);
                else if (transfer->downloadLimit(Transfer::VisibleSpeedLimit) < m_downloadLimit / n 
                                            && transfer->downloadLimit(Transfer::VisibleSpeedLimit) != 0)
                        /*If the transfer's visible download limit is under the new one, 
                                       we move the KiB/s which are different to the pool*/
                    pool = pool + (m_downloadLimit / n - transfer->downloadLimit(Transfer::VisibleSpeedLimit));       
                else if (transfer->downloadSpeed() + 10 < m_downloadLimit / n)
                {
                        /*When the downloadSpeed of the transfer is under the new downloadLimit + 10 then we 
                            set the downloadLimit to the downloadSpeed + 10*/
                    pool = pool + m_downloadLimit / n - transfer->downloadSpeed() + 10;
                    transfer->setDownloadLimit(transfer->downloadSpeed() + 10, Transfer::InvisibleSpeedLimit);
                }
                else
                {
                    transfer->setDownloadLimit(m_downloadLimit / n, Transfer::InvisibleSpeedLimit);
                    transfersNeedSpeed.append(transfer);
                }
            }
        }
        foreach (Transfer *transfer, transfersNeedSpeed)
        {
            transfer->setDownloadLimit(m_downloadLimit / n + pool / transfersNeedSpeed.count(), Transfer::InvisibleSpeedLimit);
        }
    }
}

void TransferGroup::calculateUploadLimit()
{
    kDebug(5001) << "Calculate new Upload Limit of " + QString::number(m_uploadLimit);
    if (supportsSpeedLimits())
    {
        const QList<Job*> running = runningJobs();
        int n = running.count();
        int pool = 0;//We create a pool where we have some KiB/s to go to other transfer's...
        QList<Transfer*> transfersNeedSpeed;
        foreach (Job *job, running) {
            Transfer * transfer = static_cast<Transfer*>(job);
            if (transfer)
            {
                if (m_uploadLimit == 0 && transfer->uploadLimit(Transfer::VisibleSpeedLimit) != 0)
                    continue;
                else if (m_uploadLimit == 0 && transfer->uploadLimit(Transfer::VisibleSpeedLimit) == 0)
                    transfer->setUploadLimit(0, Transfer::InvisibleSpeedLimit);
                else if (transfer->uploadLimit(Transfer::VisibleSpeedLimit) < m_uploadLimit / n 
                                                        && transfer->uploadLimit(Transfer::VisibleSpeedLimit) != 0)
                        /*If the transfer's visible upload limit is under the new one, 
                                       we move the KiB/s which are different to the pool*/
                    pool = pool + (m_uploadLimit / n - transfer->uploadLimit(Transfer::VisibleSpeedLimit));       
                else if (transfer->uploadSpeed() + 10 < m_uploadLimit / n)
                {
                        /*When the uploadSpeed of the transfer is under the new uploadLimit + 10 then we 
                            set the uploadLimit to the uploadSpeed + 10*/
                    pool = pool + m_uploadLimit / n - transfer->uploadSpeed() + 10;
                    transfer->setUploadLimit(transfer->uploadSpeed() + 10, Transfer::InvisibleSpeedLimit);
                }
                else
                {
                    transfer->setUploadLimit(m_uploadLimit / n, Transfer::InvisibleSpeedLimit);
                    transfersNeedSpeed.append(transfer);
                }
            }
        }
        foreach (Transfer *transfer, transfersNeedSpeed)
        {
            transfer->setUploadLimit(m_uploadLimit / n + pool / transfersNeedSpeed.count(), Transfer::InvisibleSpeedLimit);
        }
    }
}

void TransferGroup::save(QDomElement e) // krazy:exclude=passbyvalue
{
    //kDebug(5001) << " -->  " << name();

    e.setAttribute("Name", m_name);
    e.setAttribute("DefaultFolder", m_defaultFolder);
    e.setAttribute("DownloadLimit", m_visibleDownloadLimit);
    e.setAttribute("UploadLimit", m_visibleUploadLimit);
    e.setAttribute("Icon", m_iconName);
    e.setAttribute("Status", status() == JobQueue::Running ? "Running" : "Stopped");
    e.setAttribute("RegExpPattern", m_regExp.pattern());

#ifdef HAVE_NEPOMUK
    QDomElement tags = e.ownerDocument().createElement("Tags");
    foreach(const Nepomuk::Tag &tagOfList, m_tags)
    {
        QDomElement tag = e.ownerDocument().createElement("Tag");
        QDomText text = e.ownerDocument().createTextNode(KUrl(tagOfList.resourceUri()).url());
        tag.appendChild(text);
        tags.appendChild(tag);
    }
    e.appendChild(tags);
#endif //HAVE_NEPOMUK

    iterator it = begin();
    iterator itEnd = end();

    for( ; it!=itEnd; ++it )
    {
        Transfer* transfer = static_cast<Transfer*>(*it);
        kDebug(5001) << "  -->  " << name() << "  transfer: " << transfer->source();
        QDomElement t = e.ownerDocument().createElement("Transfer");
        e.appendChild(t);
        transfer->save(t);
    }
}

void TransferGroup::load(const QDomElement & e)
{
    kDebug(5001) << "TransferGroup::load";

    m_name = e.attribute("Name");
    m_defaultFolder = e.attribute("DefaultFolder");
    m_visibleDownloadLimit = e.attribute("DownloadLimit").toInt();
    m_visibleUploadLimit = e.attribute("UploadLimit").toInt();
    if (!e.attribute("Icon").isEmpty())
        m_iconName = e.attribute("Icon");

    if (e.attribute("Status") == "Running")
        setStatus(JobQueue::Running);
    else
        setStatus(JobQueue::Stopped);

    m_regExp.setPattern(e.attribute("RegExpPattern"));

#ifdef HAVE_NEPOMUK
    QDomNodeList tagsNodeList = e.elementsByTagName("Tags").at(0).toElement().elementsByTagName("Tag");
    for( uint i = 0; i < tagsNodeList.length(); ++i )
    {
        QString tag = tagsNodeList.item(i).toElement().text();
        if (!tag.isEmpty())
        {
            m_tags << Nepomuk::Tag(tag);
        }
    }
#endif //HAVE_NEPOMUK

    QDomNodeList nodeList = e.elementsByTagName("Transfer");
    int nItems = nodeList.length();

    QList<QDomElement> elements;
    for (int i = 0; i < nItems; ++i) {
        elements << nodeList.item(i).toElement();
    }

    kDebug(5001) << "TransferGroup::load ->" << "add" << nItems << "transfers";
    KGet::addTransfers(elements, name());
}
