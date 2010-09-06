/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
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

#include <QtDBus/QDBusConnectionInterface>
#include <KDebug>

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
}

KGetEngine::~KGetEngine()
{
}

QStringList KGetEngine::sources() const
{
    QStringList sources;
    sources << "KGet";

    return sources;
}

bool KGetEngine::sourceRequestEvent(const QString &name)
{
    return updateSourceEvent(name);
}

bool KGetEngine::updateSourceEvent(const QString &name)
{
    kDebug();
    if (name == "KGet") {
        getKGetData(name);
    }
    return true;
}

void KGetEngine::updateData()
{
    updateSourceEvent("KGet");
}

void KGetEngine::getKGetData(const QString &name)
{
    removeAllData(name);

    if (isDBusServiceRegistered()) {
        if (!m_kget) {
            m_kget = new OrgKdeKgetMainInterface(KGET_DBUS_SERVICE, KGET_DBUS_PATH, QDBusConnection::sessionBus(), this);
            connect(m_kget, SIGNAL(transfersAdded(QStringList,QStringList)), this, SLOT(slotTransfersAdded(QStringList,QStringList)));
            connect(m_kget, SIGNAL(transfersRemoved(QStringList,QStringList)), this, SLOT(slotTransfersRemoved(QStringList,QStringList)));
        }

        setData(name, "error", false);
        setData(name, "transfers", m_kget->transfers().value());
    } else {
        setData(name, "error", true);
        setData(name, "errorMessage", i18n("Is KGet up and running?"));
    }
}

void KGetEngine::transferAdded(const QString &url, const QString &dBusObjectPath)
{
    const QString name = "KGet";
    removeAllData(name);

    QVariantMap added;
    added.insert(url, dBusObjectPath);

    setData(name, "error", false);
    setData(name, "transfers", m_kget->transfers().value());
    setData(name, "transferAdded", added);
}

//TODO investigate if this should be improved for speed reasons
void KGetEngine::slotTransfersAdded(const QStringList &urls, const QStringList &dBusObjectPaths)
{
    for (int i = 0; i < urls.count(); ++i) {
        transferAdded(urls[i], dBusObjectPaths[i]);
    }
}

void KGetEngine::transferRemoved(const QString &url, const QString &dBusObjectPath)
{
    const QString name = "KGet";
    removeAllData(name);

    QVariantMap removed;
    removed.insert(url, dBusObjectPath);

    setData(name, "error", false);
    setData(name, "transfers", m_kget->transfers().value());
    setData(name, "transferRemoved", removed);
}

//TODO investigate if this should be improved for speed reasons
void KGetEngine::slotTransfersRemoved(const QStringList &urls, const QStringList &dBusObjectPaths)
{
    for (int i = 0; i < urls.count(); ++i) {
        transferRemoved(urls[i], dBusObjectPaths[i]);
    }
}

bool KGetEngine::isDBusServiceRegistered()
{
    return interface->isServiceRegistered(KGET_DBUS_SERVICE);
}

#include "kgetengine.moc"
