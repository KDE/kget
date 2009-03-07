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
#include "core/observer.h"
#include "settings.h"
#include "mainwindow.h"

#include <kdebug.h>
#include <klocale.h>
#include <KPassivePopup>
#include <QAction>
#include <QVariant>

#ifdef HAVE_KWORKSPACE
    #include <kworkspace/kworkspace.h>
#endif

TransferHandler::TransferHandler(Transfer * transfer, Scheduler * scheduler)
  : Handler(scheduler),
    m_transfer(transfer)
{
    addObserver(0);
}

TransferHandler::~TransferHandler()
{
//    qDeleteAll(m_observers);
}

void TransferHandler::addObserver(TransferObserver * observer)
{
    m_observers.push_back(observer);
    m_changesFlags[observer]=0xFFFFFFFF;
    kDebug(5001) << "TransferHandler: OBSERVERS++ = " << m_observers.size();
}

void TransferHandler::delObserver(TransferObserver * observer)
{
    m_observers.removeAll(observer);
    m_changesFlags.remove(observer);
    kDebug(5001) << "TransferHandler: OBSERVERS-- = " << m_observers.size();
}

void TransferHandler::start()
{
    if(m_transfer->group()->status() == JobQueue::Running)
    {
        m_transfer->setPolicy(Job::None);
        m_transfer->group()->move(m_transfer, 0);
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

bool TransferHandler::isResumable() const
{
    return m_transfer->isResumable();
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
            if (downloadSpeed()==0)
            {
                if (status() == Job::Running)
                    return i18n("Stalled");
                else
                    return QString();
            }
            else
                return i18n("%1/s", KIO::convertSize(downloadSpeed()));
        case 5:
            if (status() == Job::Running)
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

Transfer::ChangesFlags TransferHandler::changesFlags(TransferObserver * observer) const
{
    if( m_changesFlags.find(observer) != m_changesFlags.end() )
        return m_changesFlags[observer];
    else
    {
        kDebug(5001) << " TransferHandler::changesFlags() doesn't see you as an observer! ";

        return 0xFFFFFFFF;
    }
}

void TransferHandler::resetChangesFlags(TransferObserver * observer)
{
    if( m_changesFlags.find(observer) != m_changesFlags.end() )
        m_changesFlags[observer] = 0;
    else
        kDebug(5001) << " TransferHandler::resetChangesFlags() doesn't see you as an observer! ";
}

void TransferHandler::setTransferChange(ChangesFlags change, bool postEvent)
{
    QMap<TransferObserver *, ChangesFlags>::Iterator it = m_changesFlags.begin();
    QMap<TransferObserver *, ChangesFlags>::Iterator itEnd = m_changesFlags.end();

    for( ; it!=itEnd; ++it )
        *it |= change;

    if(postEvent)
        postTransferChangedEvent();
}

void TransferHandler::postTransferChangedEvent()
{
//     kDebug(5001) << "TransferHandler::postTransferChangedEvent() ENTERING";
    
    // Here we have to copy the list and iterate on the copy itself, because
    // a view can remove itself as a view while we are iterating over the
    // observers list and this leads to crashes.
    QList<TransferObserver *> observers = m_observers;

    QList<TransferObserver *>::iterator it = observers.begin();
    QList<TransferObserver *>::iterator itEnd = observers.end();

    // Notify the observers
    for(; it!=itEnd; ++it)
    {
        if(*it)
            (*it)->transferChangedEvent(this);
    }

    // Notify the group
    m_transfer->group()->transferChangedEvent(m_transfer);

    // Notify the TransferTreeModel
    m_transfer->model()->postDataChangedEvent(this);

    //kDebug(5001) << "TransferHandler::postTransferChangedEvent() LEAVING";
}

void TransferHandler::postDeleteEvent()
{
    kDebug(5001) << "TransferHandler::postDeleteEvent() ENTERING";

    m_transfer->postDeleteEvent();//First inform the transfer itself

    //Here we have to copy the list and iterate on the copy itself, because
    //a view can remove itself as a view while we are iterating over the
    //observers list and this leads to crashes.
    QList<TransferObserver *> observers = m_observers;

    QList<TransferObserver *>::iterator it = observers.begin();
    QList<TransferObserver *>::iterator itEnd = observers.end();

    for(; it!=itEnd; ++it)
    {
        if(*it && *it != dynamic_cast<TransferObserver*>(m_transfer))
            (*it)->deleteEvent(this);
    }
    kDebug(5001) << "TransferHandler::postDeleteEvent() LEAVING";
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
            << KGet::actionCollection()->action("redownload_selected_download");

    return actions;
}

QList<QAction*> TransferHandler::factoryActions()
{
    QList<QAction*> actions;
    foreach(QAction *action, m_transfer->factory()->actions(this))
        actions.append(action);
    return actions;
}


GenericTransferObserver::GenericTransferObserver(GenericTransferGroupObserver *groupObserver)
  : QObject(groupObserver),
    TransferObserver()
{
    m_groupObserver = groupObserver;
}

void GenericTransferObserver::transferChangedEvent(TransferHandler * transfer)
{
    //kDebug(5001);
    TransferHandler::ChangesFlags transferFlags = transfer->changesFlags(this);

    if (transfer->status() == Job::Finished && Settings::afterFinishActionEnabled() 
                                            && Settings::afterFinishAction() == KGet::Quit) 
    {
        checkAndFinish();
    }

#ifdef HAVE_KWORKSPACE
    if (transfer->status() == Job::Finished && Settings::afterFinishActionEnabled() 
                                            && Settings::afterFinishAction() == KGet::Shutdown) {
        checkAndShutdown();
    }
#endif

    if (prevStatus != transfer->statusText())//FIXME: HACK: better: check statusFlags if it 
    {                                                                 //contains Tc_Status (flags & Transfer::Tc_Status <-doesn't work)
        prevStatus = transfer->statusText();
        KGet::checkSystemTray();
    }

    if (transferFlags & Transfer::Tc_Percent) {
        transfer->group()->setGroupChange(TransferGroup::Gc_Percent, true);
    }

    if (transferFlags & Transfer::Tc_DownloadSpeed) {
        transfer->group()->setGroupChange(TransferGroup::Gc_DownloadSpeed, true);
    }

    if (transferFlags & Transfer::Tc_UploadSpeed) {
        transfer->group()->setGroupChange(TransferGroup::Gc_UploadSpeed, true);
    }

    transfer->resetChangesFlags(this);
    transfer->checkShareRatio();

    m_groupObserver->postTransferChanged(transfer);
}

bool GenericTransferObserver::allTransfersFinished()
{
    bool quitFlag = true;
    foreach(TransferGroup *transferGroup, KGet::m_transferTreeModel->transferGroups()) {
        foreach(TransferHandler *transfer, transferGroup->handler()->transfers()) {
            if(transfer->status() != Job::Finished) {
                quitFlag = false;
            }
        }
    }

    return quitFlag;
}

KPassivePopup* GenericTransferObserver::popupMessage(const QString &title, const QString &message)
{
    KPassivePopup *popup;
    // we have to call diferent message from kpassivePopup
    // one with parent as QWidget for the mainWindow
    // and another with parent as QSystemTrayIcon if the parent is a systemTray
    // so passing the QSystemTrayIcon as QWidget don't work
    if(Settings::enableSystemTray()) 
    {
        popup = KPassivePopup::message(5000, title, message, KGet::m_mainWindow->systemTray());
    }
    else 
    {
        popup = KPassivePopup::message(5000, title, message, KGet::m_mainWindow);
    }

    return popup;
}

void GenericTransferObserver::checkAndFinish()
{
    // check if there is some unfinished transfer in scheduler queues
    if(allTransfersFinished()) {
        KPassivePopup *message = popupMessage(i18n("Quit KGet"),
                                            i18n("KGet is now closing, as all downloads have completed."));
        QObject::connect(message, SIGNAL(destroyed()), KGet::m_mainWindow, SLOT(slotQuit()));
    }
}

#ifdef HAVE_KWORKSPACE
void GenericTransferObserver::checkAndShutdown()
{
    if(allTransfersFinished()) {
        KPassivePopup *message = popupMessage(i18n("Quit KGet"),
                                            i18n("The computer will now turn off, as all downloads have completed."));
        QObject::connect(message, SIGNAL(destroyed()), SLOT(slotShutdown()));
        QObject::connect(message, SIGNAL(destroyed()),  KGet::m_mainWindow, SLOT(slotQuit()));
    }
}

void GenericTransferObserver::slotShutdown()
{
    KWorkSpace::requestShutDown(KWorkSpace::ShutdownConfirmNo,
                                KWorkSpace::ShutdownTypeHalt,
                                KWorkSpace::ShutdownModeForceNow);
}
#endif
