/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   Copyright (C) 2013 by Bhushan Shah <bhush94@gmail.com>
 *   Copyright (C) 2007 by Javier Goday <jgoday@gmail.com>
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "kgetengine.h"
#include "kget_interface.h"
#include "transfer_interface.h"

#include <QtDBus/QDBusConnectionInterface>
#include <KDebug>

#include <kmimetype.h>
#include "plasma/datacontainer.h"


const quint16 KGetEngine::MINIMUM_UPDATE_INTERVAL = 1000;
const QString KGetEngine::KGET_DBUS_SERVICE = "org.kde.kget";
const QString KGetEngine::KGET_DBUS_PATH = "/KGet";

KGetEngine::KGetEngine(QObject* parent, const QVariantList& args)
  : Plasma::DataEngine(parent),
    m_kget(0)
{
    Q_UNUSED(args)

    interface = QDBusConnection::sessionBus().interface();
    setMinimumPollingInterval(MINIMUM_UPDATE_INTERVAL);
    setName("KGet");
    setIcon("kget");
}

KGetEngine::~KGetEngine()
{
}

void KGetEngine::init()
{
    getKGetData();
}

bool KGetEngine::sourceRequestEvent(const QString &name)
{
    return updateSourceEvent(name);
}

bool KGetEngine::updateSourceEvent(const QString &name)
{
    Q_UNUSED(name)

    getKGetData();
    return true;
}

void KGetEngine::updateData()
{
    updateSourceEvent(QString());
}

void KGetEngine::getKGetData()
{
    Plasma::DataEngine::Data data;

    if (isDBusServiceRegistered()) {
        if (!m_kget) {
            m_kget = new OrgKdeKgetMainInterface(KGET_DBUS_SERVICE, KGET_DBUS_PATH, QDBusConnection::sessionBus(), this);
        }
    }
    else {
        removeAllSources();
        data.insert("message", "KGet is not running");
        setData("Error",data);
        return;
    }

    removeSource("Error");
    QVariantMap transfer = m_kget->transfers().value();
    for (QVariantMap::const_iterator it = transfer.constBegin(); it != transfer.constEnd(); ++it) {
        OrgKdeKgetTransferInterface *newTransfer = new OrgKdeKgetTransferInterface("org.kde.kget", it.value().toString(), QDBusConnection::sessionBus(), this);
        data.clear();
        data.insert("filename", KUrl(newTransfer->dest()).fileName());
        data.insert("status", newTransfer->statusText().value());
        data.insert("progress", newTransfer->percent().value());
        data.insert("dest", newTransfer->dest().value());
        data.insert("size", newTransfer->totalSize().value());
        data.insert("src", newTransfer->source().value());
        data.insert("speed", newTransfer->downloadSpeed().value());
        data.insert("icon", KMimeType::iconNameForUrl(KUrl(newTransfer->dest().value())));
        setData(KUrl(newTransfer->dest()). fileName(),data);
    }

}

bool KGetEngine::isDBusServiceRegistered()
{
    return interface->isServiceRegistered(KGET_DBUS_SERVICE);
}

#include "kgetengine.moc"
