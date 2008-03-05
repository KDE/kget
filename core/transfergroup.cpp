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

#include <KDebug>
#include <KMessageBox>
#include <KLocale>
#include <KStandardDirs>
#include <kio/global.h>

#include <QDomElement>
#include <QFile>
#include <QDateTime>


TransferGroup::TransferGroup(TransferTreeModel * model, Scheduler * scheduler, const QString & name)
    : JobQueue(scheduler),
      m_model(model), m_name(name),
      m_totalSize(0), m_processedSize(0),
      m_percent(0), m_speed(0),
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

    QString filename = KStandardDirs::locateLocal("appdata", "transferhistory.kgt");
    QFile file(filename);
    QDomDocument *doc;
    QDomElement root;

    if (!file.exists())
    {
        doc = new QDomDocument("Transfers");
        root = doc->createElement("Transfers");
        doc->appendChild(root);
    }
    else
    {
        doc = new QDomDocument();
        doc->setContent(&file);
        file.close();
        root = doc->documentElement();
        doc->appendChild(root);
    }

    QDomElement e = doc->createElement("Transfer");
    root.appendChild(e);

    e.setAttribute("Source", transfer->source().url());
    e.setAttribute("Dest", transfer->dest().url());
    e.setAttribute("Time", QDateTime::currentDateTime().toString());
    e.setAttribute("Size", QString::number(transfer->totalSize()));

    kDebug(5001) << transfer->statusText();

    if (transfer->statusText() == "Finished")
        e.setAttribute("State", i18nc("the transfer has been finished", "Finished"));
    else
        e.setAttribute("State", i18nc("the transfer has been aborted", "Aborted"));

    if (!file.open(QIODevice::ReadWrite))
        KMessageBox::error(0, i18n("History-File cannot be opened correctly!"), i18n("Error"), 0);

    QTextStream stream( &file );
    doc->save( stream, 0 );
    file.close();
    kDebug(5001) << "remove Transfer:" << transfer->source().url();

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
    setUploadLimit(m_visibleUlLimit);
}

void TransferGroup::setVisibleDownloadLimit(int limit) 
{
    m_visibleDlLimit = limit;
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
    kDebug(5001);//FIXME: Perhaps anyone has a better calculation/algorithm for that and calculateUploadLimit() ?
    if (supportsSpeedLimits())
    {
        int n = runningJobs().count();
        foreach (Job * job, runningJobs())
        {
            Transfer * transfer = static_cast<Transfer*>(job);
            if (transfer)
            {
                if (visibleDownloadLimit() < downloadLimit() && transfer->visibleDownloadLimit() < downloadLimit())
                    transfer->setDownloadLimit(visibleDownloadLimit() / n);
                else if (visibleDownloadLimit() < downloadLimit())
                    transfer->setDownloadLimit(downloadLimit() / n);
            }
        }
    }
}

void TransferGroup::calculateUploadLimit()
{
    kDebug(5001);
    if (supportsSpeedLimits())
    {
        int n = runningJobs().count();
        foreach (Job * job, runningJobs())
        {
            Transfer * transfer = static_cast<Transfer*>(job);
            if (transfer)
            {
                if (visibleUploadLimit() < uploadLimit())
                    transfer->setUploadLimit(visibleUploadLimit() / n);
                else
                    transfer->setUploadLimit(uploadLimit() / n);
            }
        }
    }
}

void TransferGroup::transferChangedEvent(Transfer * transfer)
{
    Q_UNUSED(transfer);
//     Disable this line for now, since as of now we don't do nothing with this event.
//     m_handler->postGroupChangedEvent();
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
