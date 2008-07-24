/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *
 *   Copyright (C) 2008 by Lukas Appelhans <l.appelhans@gmx.de>
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
#include "kgetapplet.h"
#include "kget_interface.h"

#include <plasma/dataengine.h>

#include <QGraphicsSceneDragDropEvent>
#include <QDropEvent>
#include <QtDBus/QDBusConnectionInterface>
#include <KUrl>

KGetApplet::KGetApplet(QObject *parent, const QVariantList &args)
  : Plasma::Applet(parent, args),
    m_engine(0)
{
    setAcceptDrops(true);
}

KGetApplet::~KGetApplet()
{
}

void KGetApplet::init()
{
    m_engine = dataEngine("kget");
    if (m_engine) {
        m_engine->connectSource("KGet", this);
        m_engine->setProperty("refreshTime", 6000);
    }
    else {
        kDebug() << "KGet Engine could not be loaded";
    }
}

bool KGetApplet::sceneEventFilter(QGraphicsItem * watched, QEvent * event)
{
    Q_UNUSED(watched);
    switch (event->type())
    {
        case QEvent::GraphicsSceneDrop:
            dropEvent(static_cast<QGraphicsSceneDragDropEvent*>(event));
            return true;
        case QEvent::Drop:
            dropEvent(static_cast<QDropEvent*>(event));
            return true;
        default:
            break;
    }
    return false;
}

void KGetApplet::dropEvent(QGraphicsSceneDragDropEvent * event)
{
    kDebug();
    if (event->mimeData()->hasUrls() && QDBusConnection::sessionBus().interface()->isServiceRegistered(KGET_DBUS_SERVICE))
    {
        OrgKdeKgetInterface kget_interface(KGET_DBUS_SERVICE, KGET_DBUS_PATH,
                            QDBusConnection::sessionBus());
        QStringList urls;
        foreach (const KUrl &url, event->mimeData()->urls())
            urls.append(url.url());

        kget_interface.showNewTransferDialog(urls);
        event->accept();
    }
}

void KGetApplet::dropEvent(QDropEvent * event)
{
    kDebug();
    if (event->mimeData()->hasUrls() && QDBusConnection::sessionBus().interface()->isServiceRegistered(KGET_DBUS_SERVICE))
    {
        OrgKdeKgetInterface kget_interface(KGET_DBUS_SERVICE, KGET_DBUS_PATH,
                            QDBusConnection::sessionBus());
        QStringList urls;
        foreach (const KUrl &url, event->mimeData()->urls())
            urls.append(url.url());

        kget_interface.showNewTransferDialog(urls);
        event->accept();
    }
}

#include "kgetapplet.moc"
