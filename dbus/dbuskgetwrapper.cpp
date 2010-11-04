/**************************************************************************
*   Copyright (C) 2006 - 2008 Urs Wolfer <uwolfer @ kde.org>              *
*   Copyright (C) 2006 Dario Massarin <nekkar@libero.it>                  *
*   Copyright (C) 2008 - 2009 Lukas Appelhans <l.appelhans@gmx.de>        *
*   Copyright (C) 2009 Matthias Fuchs <mat69@gmx.net>                     *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
***************************************************************************/

#include "dbuskgetwrapper.h"

#include "core/kget.h"
#include "core/transferhandler.h"
#include "core/transfertreemodel.h"
#include "core/plugin/transferfactory.h"
#include "mainwindow.h"
#include "settings.h"
#include "ui/droptarget.h"
#include "ui/linkview/kget_linkview.h"
#include "ui/newtransferdialog.h"

#include <KDebug>

DBusKGetWrapper::DBusKGetWrapper(MainWindow *parent)
  : QObject(parent),
    m_mainWindow(parent)
{
    foreach (TransferHandler *handler, KGet::allTransfers()) {
        m_transfers[handler] = qMakePair(handler->source().pathOrUrl(), handler->dBusObjectPath());
    }

    TransferTreeModel *model = KGet::model();

    connect(model, SIGNAL(transfersAddedEvent(QList<TransferHandler*>)), this, SLOT(slotTransfersAdded(QList<TransferHandler*>)));
    connect(model, SIGNAL(transfersRemovedEvent(QList<TransferHandler*>)), this, SLOT(slotTransfersRemoved(QList<TransferHandler*>)));
}

DBusKGetWrapper::~DBusKGetWrapper()
{
}

QStringList DBusKGetWrapper::addTransfer(const QString& src, const QString& dest, bool start)
{
    QStringList dBusPaths;


    // split src for the case it is a QStringList (e.g. from konqueror plugin)
    QList<TransferHandler*> addedTransfers = KGet::addTransfer(src.split(';'), dest, QString(), start);

    foreach (TransferHandler *handler, addedTransfers) {
        dBusPaths.append(handler->dBusObjectPath());
    }

    return dBusPaths;
}

bool DBusKGetWrapper::delTransfer(const QString& dbusObjectPath)
{
    kDebug(5001) << "deleting Transfer";

    Transfer *transfer = KGet::model()->findTransferByDBusObjectPath(dbusObjectPath);

    if (transfer) {
        return KGet::delTransfer(transfer->handler());
    }

    return false;
}

void DBusKGetWrapper::showNewTransferDialog(const QStringList &urls)
{
    NewTransferDialogHandler::showNewTransferDialog(urls);
}

bool DBusKGetWrapper::dropTargetVisible() const
{
    return m_mainWindow->m_drop->isVisible();
}

void DBusKGetWrapper::setDropTargetVisible(bool setVisible)
{
    if (setVisible != Settings::showDropTarget()) {
        m_mainWindow->m_drop->setDropTargetVisible(setVisible);
    }
}

void DBusKGetWrapper::setOfflineMode(bool offline)
{
    KGet::setSchedulerRunning(offline);
}

bool DBusKGetWrapper::offlineMode() const
{
    return !KGet::schedulerRunning();
}

QVariantMap DBusKGetWrapper::transfers() const
{
    const QList<QPair<QString, QString> > transfers = m_transfers.values();
    QVariantMap result;
    for (int i = 0; i < transfers.count(); ++i) {
        result.insert(transfers[i].first, transfers[i].second);
    }

    return result;
}

void DBusKGetWrapper::slotTransfersAdded(const QList<TransferHandler*> &transfers)
{
    QStringList urls;
    QStringList objectPaths;
    foreach (TransferHandler *transfer, transfers) {
        const QString url = transfer->source().pathOrUrl();
        const QString objectPath = transfer->dBusObjectPath();
        urls << url;
        objectPaths << objectPath;
        m_transfers[transfer] = qMakePair(url, objectPath);
    }

    emit transfersAdded(urls, objectPaths);
}

void DBusKGetWrapper::slotTransfersRemoved(const QList<TransferHandler*> &transfers)
{
    QStringList urls;
    QStringList objectPaths;
    foreach (TransferHandler *transfer, transfers) {
        const QPair<QString, QString> removed = m_transfers[transfer];
        urls << removed.first;
        objectPaths << removed.second;
    }

    emit transfersRemoved(urls, objectPaths);
}

int DBusKGetWrapper::transfersSpeed() const
{
    return 0;//FIXME
    //return m_dbusModelObserver->transfersSpeed();
}

void DBusKGetWrapper::importLinks(const QList <QString> &links)
{
    KGetLinkView *link_view = new KGetLinkView(m_mainWindow);
    link_view->setLinks(links);
    link_view->show();
}

bool DBusKGetWrapper::isSupported(const QString &url) const
{
    foreach (TransferFactory * factory, KGet::factories()) {
        kDebug() << "Check" << factory->objectName() << "for" << url << "it is?" << factory->isSupported(KUrl(url));
        if (factory->isSupported(KUrl(url)))
            return true;
    }
    return false;
}

#include "dbuskgetwrapper.moc"
