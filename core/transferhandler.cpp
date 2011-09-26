/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "core/transferhandler.h"

#include "core/job.h"
#include "core/jobqueue.h"
#include "core/transfergroup.h"
#include "core/transfergrouphandler.h"
#include "core/transfertreemodel.h"
#include "core/plugin/transferfactory.h"
#include "settings.h"
#include "mainwindow.h"
#include "kgetkjobadapter.h"

#include <kdebug.h>
#include <klocale.h>
#include <KPassivePopup>
#include <QAction>
#include <QVariant>

TransferHandler::TransferHandler(Transfer * parent, Scheduler * scheduler)
  : Handler(scheduler, parent),
    m_transfer(parent),
    m_changesFlags(Transfer::Tc_None)
{
    static int dBusObjIdx = 0;
    m_dBusObjectPath = "/KGet/Transfers/" + QString::number(dBusObjIdx++);

    m_kjobAdapter = new KGetKJobAdapter(this, this);

    connect(m_transfer, SIGNAL(capabilitiesChanged()), this, SIGNAL(capabilitiesChanged()));
}

TransferHandler::~TransferHandler()
{
}

Transfer::Capabilities TransferHandler::capabilities() const
{
    return m_transfer->capabilities();
}

void TransferHandler::start()
{
    if(m_transfer->group()->status() == JobQueue::Running)
    {
        m_transfer->setPolicy(Job::None);
        KGet::model()->moveTransfer(m_transfer, m_transfer->group());
    }
    else
    {
        m_transfer->setPolicy(Job::Start);
    }
}

void TransferHandler::stop()
{
    if (m_transfer->group()->status() == JobQueue::Stopped)
    {
        m_transfer->setPolicy(Job::None);
    }
    else
    {
        m_transfer->setPolicy(Job::Stop);
    }
}

int TransferHandler::elapsedTime() const
{
    return m_transfer->elapsedTime();
}

int TransferHandler::remainingTime() const
{
    return m_transfer->remainingTime();
}

KIO::filesize_t TransferHandler::totalSize() const
{
    return m_transfer->totalSize();
}

KIO::filesize_t TransferHandler::downloadedSize() const
{
    return m_transfer->downloadedSize();
}

KIO::filesize_t TransferHandler::uploadedSize() const
{
    return m_transfer->uploadedSize();
}

int TransferHandler::percent() const
{
    return m_transfer->percent();
}

int TransferHandler::downloadSpeed() const
{
    return m_transfer->downloadSpeed();
}

int TransferHandler::averageDownloadSped() const
{
    return m_transfer->averageDownloadSpeed();
}

int TransferHandler::uploadSpeed() const
{
    return m_transfer->uploadSpeed();
}

QVariant TransferHandler::data(int column)
{
//     kDebug(5001) << "TransferHandler::data(" << column << ")";

    switch(column)
    {
        case 0:
            return dest().fileName();
        case 1:
            if (status() == Job::Aborted && !error().text.isEmpty())
                return error().text;
            return statusText();
        case 2:
            if (totalSize() != 0)
                return KIO::convertSize(totalSize());
            else
                return i18nc("not available", "n/a");
        case 3:
//             return QString::number(percent())+'%'; // display progressbar instead
            return QVariant();
        case 4:
            if (downloadSpeed() == 0)
            {
                if (m_transfer->isStalled())
                    return i18n("Stalled");
                else
                    return QString();
            }
            else
                return i18n("%1/s", KIO::convertSize(downloadSpeed()));
        case 5:
            if (status() == Job::Running && downloadSpeed() != 0)
                return KIO::convertSeconds(remainingTime());
            else
                return QString();
        default:
            return QVariant();
    }
}

void TransferHandler::setSelected( bool select )
{
    if( (select && !isSelected()) || (!select && isSelected()) )
    {
        m_transfer->m_isSelected = select;
        setTransferChange( Transfer::Tc_Selection, true );
    }
}

bool TransferHandler::isSelected() const
{
    return m_transfer->m_isSelected;
}

Transfer::ChangesFlags TransferHandler::changesFlags() const
{
    return m_changesFlags;
}

void TransferHandler::resetChangesFlags()
{
    m_changesFlags = 0;
}

void TransferHandler::destroy()
{
    kDebug(5001) << "TransferHandler::destroy() ENTERING";

    kDebug(5001) << "TransferHandler::destroy() LEAVING";
}

void TransferHandler::setTransferChange(ChangesFlags change, bool notifyModel)
{
    m_changesFlags |= change;

    if (notifyModel)
    {
        // Notify the TransferTreeModel
        m_transfer->model()->postDataChangedEvent(this);
    
        m_kjobAdapter->slotUpdateDescription();
    }
}

QList<QAction*> TransferHandler::contextActions()
{
    QList<QAction*> actions;
    if (status() != Job::Finished) 
    {
        actions << KGet::actionCollection()->action("start_selected_download")
                << KGet::actionCollection()->action("stop_selected_download");
    }
    actions << KGet::actionCollection()->action("delete_selected_download")
            << KGet::actionCollection()->action("redownload_selected_download")
            << KGet::actionCollection()->action("select_all");

    return actions;
}

QList<QAction*> TransferHandler::factoryActions()
{
    QList<QAction*> actions;
    foreach(QAction *action, m_transfer->factory()->actions(this))
        actions.append(action);
    return actions;
}
