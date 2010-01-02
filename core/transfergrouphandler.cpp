/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2008 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "core/transfergrouphandler.h"

#include "core/kgetkjobadapter.h"
#include "core/transferhandler.h"
#include "core/transfertreemodel.h"
#include "core/transfer.h"
#include "core/kget.h"

#include <kdebug.h>
#include <kmenu.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <klocale.h>
#include <kicon.h>

TransferGroupHandler::TransferGroupHandler(Scheduler * scheduler, TransferGroup * parent)
  : Handler(scheduler, parent),
    m_group(parent),
    m_changesFlags(Transfer::Tc_None)
{
}

TransferGroupHandler::~TransferGroupHandler()
{
}

void TransferGroupHandler::start()
{
    kDebug(5001) << "TransferGroupHandler::start()";
    m_group->setStatus( JobQueue::Running );
}

void TransferGroupHandler::stop()
{
    kDebug(5001) << "TransferGroupHandler::stop()";
    m_group->setStatus( JobQueue::Stopped );
}

void TransferGroupHandler::move(QList<TransferHandler *> transfers, TransferHandler * after)
{
    //Check that the given transfer (after) belongs to this group
    if( after && (after->group() != this) )
        return;

    QList<TransferHandler *>::iterator it = transfers.begin();
    QList<TransferHandler *>::iterator itEnd = transfers.end();

    for( ; it!=itEnd ; ++it )
    {
        //Move the transfers in the JobQueue
        if(after)
            m_group->move( (*it)->m_transfer, after->m_transfer );
        else
            m_group->move( (*it)->m_transfer, 0 );

        after = *it;
    }
}

TransferHandler * TransferGroupHandler::operator[] (int i)
{
//     kDebug(5001) << "TransferGroupHandler::operator[" << i << "]";

    return (*m_group)[i]->handler();
}

void TransferGroupHandler::setName(const QString &name)
{
    m_group->setName(name);
}

QVariant TransferGroupHandler::data(int column)
{
//     kDebug(5001) << "TransferGroupHandler::data(" << column << ")";

    switch(column)
    {
        case 0:
            /*if (!m_group->supportsSpeedLimits() && 
                             (m_group->downloadLimit(Transfer::VisibleSpeedLimit) != 0 || m_group->uploadLimit(Transfer::VisibleSpeedLimit) != 0))
                return name() + " - Does not supports SpeedLimits";//FIXME: Do a better text here
            else*/
                return name();
        case 2:
            if(m_group->size())
                return i18np("1 Item", "%1 Items", m_group->size());
            else
                return QString();
/*            if (totalSize() != 0)
                return KIO::convertSize(totalSize());
            else
                return i18nc("not available", "n/a");*/
        case 3:
//             return QString::number(percent())+'%'; // display progressbar instead
            return QVariant();
        case 4:
            if (downloadSpeed() == 0)
            {
                return QString();
            }
            else
                return i18n("%1/s", KIO::convertSize(downloadSpeed()));
        default:
            return QVariant();
    }
}

TransferGroup::ChangesFlags TransferGroupHandler::changesFlags()
{
    return m_changesFlags;
}

void TransferGroupHandler::resetChangesFlags()
{
    m_changesFlags = 0;
}

int TransferGroupHandler::indexOf(TransferHandler * transfer)
{
    return m_group->indexOf(transfer->m_transfer);
}

const QList<TransferHandler *> TransferGroupHandler::transfers()
{
    QList<TransferHandler *> transfers;

    TransferGroup::iterator it = m_group->begin();
    TransferGroup::iterator itEnd = m_group->end();

    for( ; it!=itEnd ; ++it )
    {
        transfers.append((static_cast<Transfer *>(*it))->handler());
    }
    return transfers;
}

const QList<QAction *> & TransferGroupHandler::actions()
{
    createActions();

    return m_actions;
}

void TransferGroupHandler::setGroupChange(ChangesFlags change, bool notifyModel)
{
    m_changesFlags |= change;

    if (notifyModel)
        m_group->model()->postDataChangedEvent(this);
}

void TransferGroupHandler::createActions()
{
    if( !m_actions.empty() )
        return;

    QAction *startAction = KGet::actionCollection()->addAction("transfer_group_start");
    startAction->setText(i18nc("start transfergroup downloads", "Start"));
    startAction->setIcon(KIcon("media-playback-start"));
    QObject::connect(startAction, SIGNAL(triggered()), SLOT(start()));
    m_actions.append(startAction);

    QAction *stopAction = KGet::actionCollection()->addAction("transfer_group_stop");
    stopAction->setText(i18nc("stop transfergroup downloads", "Stop"));
    stopAction->setIcon(KIcon("media-playback-pause"));
    QObject::connect(stopAction, SIGNAL(triggered()), SLOT(stop()));
    m_actions.append(stopAction);

}

#include "transfergrouphandler.moc"
