/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "core/transfergroup.h"

#include "core/transfer.h"
#include "core/transfergrouphandler.h"
#include "core/kget.h"
#include "core/transferhistorystore.h"

#include <KDebug>
#include <KMessageBox>
#include <KLocale>
#include <KStandardDirs>
#include <kio/global.h>

#include <QFile>
#include <QDateTime>
#include <QDomElement>


TransferGroup::TransferGroup(TransferTreeModel * model, Scheduler * scheduler, const QString & name)
    : JobQueue(scheduler),
      m_model(model), m_name(name),
      m_totalSize(0), m_downloadedSize(0), m_uploadedSize(0),
      m_percent(0), m_downloadSpeed(0), m_uploadSpeed(0),
      m_dlLimit(0), m_ulLimit(0),
      m_visibleDlLimit(0), m_visibleUlLimit(0),
      m_iconName("bookmark-new-list"), m_defaultFolder(0)
{
    m_handler = new TransferGroupHandler(this, scheduler);
}

TransferGroup::~TransferGroup()
{
    m_handler->postDeleteEvent();
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
    bool b = true;
    foreach (Job * job, runningJobs())
    {
        Transfer * transfer = static_cast<Transfer*>(job);
        if (!transfer->supportsSpeedLimits())
            b = false;
    }
    return b;
}

void TransferGroup::setStatus(Status queueStatus)
{
    JobQueue::setStatus(queueStatus);

    m_handler->setGroupChange(Gc_Status, true);
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

    calculateSpeedLimits();
    m_handler->postAddedTransferEvent(transfer, after);
}

void TransferGroup::prepend(Transfer * transfer)
{
    JobQueue::prepend(transfer);

    m_handler->postAddedTransferEvent(transfer, 0);
    calculateSpeedLimits();
}

void TransferGroup::insert(Transfer * transfer, Transfer * after)
{
    JobQueue::insert(transfer, after);

    m_handler->postAddedTransferEvent(transfer, after);
    calculateSpeedLimits();
}

void TransferGroup::remove(Transfer * transfer)
{
    TransferHistoryStore::getStore()->saveItem(TransferHistoryItem(*transfer));
    JobQueue::remove(transfer);

    m_handler->postRemovedTransferEvent(transfer);
    calculateSpeedLimits();
}

void TransferGroup::move(Transfer * transfer, Transfer * after)
{
    if(transfer == after)
        return;

    TransferGroup * oldTransferGroup = transfer->group();

    JobQueue::move(transfer, after);

    if(oldTransferGroup == this)
        m_handler->postMovedTransferEvent(transfer, after);
    else
    {
        m_handler->postAddedTransferEvent(transfer, after);
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

Transfer * TransferGroup::operator[] (int i) const
{
//     kDebug(5001) << "TransferGroup::operator[]";

    return (Transfer *)((* (JobQueue *)this)[i]);
}

TransferGroupHandler * TransferGroup::handler() const
{
    return m_handler;
}

void TransferGroup::setVisibleUploadLimit(int limit) 
{
    m_visibleUlLimit = limit;
    if (uploadLimit() > m_visibleUlLimit || uploadLimit() == 0)
        setUploadLimit(m_visibleUlLimit);
}

void TransferGroup::setVisibleDownloadLimit(int limit) 
{
    m_visibleDlLimit = limit;
    if (downloadLimit() > m_visibleDlLimit || downloadLimit() == 0)
        setDownloadLimit(m_visibleDlLimit);
}

void TransferGroup::setDownloadLimit(int limit) 
{
    m_dlLimit = limit;
    calculateDownloadLimit();
}

void TransferGroup::setUploadLimit(int limit) 
{
    m_ulLimit = limit;
    calculateUploadLimit();
}

void TransferGroup::calculateSpeedLimits()
{
    kDebug(5001) << "*************************** HERE WE ARE";
    calculateDownloadLimit();
    calculateUploadLimit();
}

void TransferGroup::calculateDownloadLimit()
{
    kDebug(5001) << "Now we are calculating download Limits =): " + QString::number(downloadLimit());
    if (supportsSpeedLimits())
    {
        kDebug(5001) << "We are supporting speedlimits =)";
        int n = runningJobs().count();
        int pool = 0;//We create a pool where we have some KiB/s to go to other transfer's...
        QList<Transfer*> transfersNeedSpeed;
        foreach (Job * job, runningJobs())
        {
            Transfer * transfer = static_cast<Transfer*>(job);
            if (transfer)
            {
                kDebug(5001) << "Cast was ok =)";
                if (downloadLimit() == 0 && transfer->visibleDownloadLimit() != 0)
                    continue;
                else if (downloadLimit() == 0 && transfer->visibleDownloadLimit() == 0)
                    transfer->setDownloadLimit(0);
                else if (transfer->visibleDownloadLimit() < downloadLimit() / n && transfer->visibleDownloadLimit() != 0)
                        /*If the transfer's visible download limit is under the new one, 
                                       we move the KiB/s which are different to the pool*/
                    pool = pool + (downloadLimit() / n - transfer->visibleDownloadLimit());       
                else if (transfer->downloadSpeed() + 10 < downloadLimit() / n)
                {
                    kDebug(5001) << "Ok the download speed man is tooooo low, so we have a small buffer";
                        /*When the downloadSpeed of the transfer is under the new downloadLimit + 10 then we 
                            set the downloadLimit to the downloadSpeed + 10*/
                    pool = pool + downloadLimit() / n - transfer->downloadSpeed() + 10;
                    transfer->setDownloadLimit(transfer->downloadSpeed() + 10);
                }
                else
                {
                    kDebug(5001) << "Ok the generic sollution xD";
                    transfer->setDownloadLimit(downloadLimit() / n);
                    transfersNeedSpeed.append(transfer);
                }
            }
        }
        foreach (Transfer *transfer, transfersNeedSpeed)
        {
            transfer->setDownloadLimit(downloadLimit() / n + pool / transfersNeedSpeed.count());
        }
    }
}

void TransferGroup::calculateUploadLimit()
{
    kDebug(5001) << "Now we are calculating upload Limits =)";
    if (supportsSpeedLimits())
    {
        kDebug(5001) << "We are supporting speedlimits =)";
        int n = runningJobs().count();
        int pool = 0;//We create a pool where we have some KiB/s to go to other transfer's...
        QList<Transfer*> transfersNeedSpeed;
        foreach (Job * job, runningJobs())
        {
            Transfer * transfer = static_cast<Transfer*>(job);
            if (transfer)
            {
                kDebug(5001) << "Cast was ok =)";
                if (uploadLimit() == 0 && transfer->visibleUploadLimit() != 0)
                    continue;
                else if (uploadLimit() == 0 && transfer->visibleUploadLimit() == 0)
                    transfer->setUploadLimit(0);
                else if (transfer->visibleUploadLimit() < uploadLimit() / n && transfer->visibleUploadLimit() != 0)
                        /*If the transfer's visible upload limit is under the new one, 
                                       we move the KiB/s which are different to the pool*/
                    pool = pool + (uploadLimit() / n - transfer->visibleUploadLimit());       
                else if (transfer->uploadSpeed() + 10 < uploadLimit() / n)
                {
                    kDebug(5001) << "Ok the upload speed man is tooooo low, so we have a small buffer";
                        /*When the uploadSpeed of the transfer is under the new uploadLimit + 10 then we 
                            set the uploadLimit to the uploadSpeed + 10*/
                    pool = pool + uploadLimit() / n - transfer->uploadSpeed() + 10;
                    transfer->setUploadLimit(transfer->uploadSpeed() + 10);
                }
                else
                {
                    kDebug(5001) << "Ok the generic sollution xD";
                    transfer->setUploadLimit(uploadLimit() / n);
                    transfersNeedSpeed.append(transfer);
                }
            }
        }
        foreach (Transfer *transfer, transfersNeedSpeed)
        {
            transfer->setUploadLimit(uploadLimit() / n + pool / transfersNeedSpeed.count());
        }
    }
}

void TransferGroup::transferChangedEvent(Transfer * transfer)
{
    Q_UNUSED(transfer);
//     Disable this line for now, since as of now we don't do nothing with this event.  
//  reenabled  for the plasma applet
    m_handler->postGroupChangedEvent();
}

void TransferGroup::save(QDomElement e) // krazy:exclude=passbyvalue
{
    kDebug(5001) << "TransferGroup::save()  -->  " << name();

    e.setAttribute("Name", m_name);
    e.setAttribute("DefaultFolder", m_defaultFolder);
    e.setAttribute("DownloadLimit", m_dlLimit);
    e.setAttribute("UploadLimit", m_ulLimit);
    e.setAttribute("Icon", m_iconName);

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
    m_defaultFolder = e.attribute("DefaultFolder");
    m_dlLimit = e.attribute("DownloadLimit").toInt();
    m_ulLimit = e.attribute("UploadLimit").toInt();
    if (!e.attribute("Icon").isEmpty())
        m_iconName = e.attribute("Icon");

    QDomNodeList nodeList = e.elementsByTagName("Transfer");
    int nItems = nodeList.length();

    for(int i=0; i<nItems; i++)
    {
        kDebug(5001) << "TransferGroup::load -> addTransfer";
        KGet::addTransfer( nodeList.item(i).toElement(), name() );
    }
}
