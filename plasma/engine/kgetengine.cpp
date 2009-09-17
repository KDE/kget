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

KGetEngine::KGetEngine(QObject* parent, const QVariantList& args)
    : Plasma::DataEngine(parent)
{
    Q_UNUSED(args);

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
    if (QString::compare(name, "KGet") == 0) {
        getKGetData(name);
    }
    return true;
}

void KGetEngine::getKGetData(const QString &name)
{
    removeAllData(name);

    if(isDBusServiceRegistered()) {
        OrgKdeKgetMainInterface kget_interface(KGET_DBUS_SERVICE, KGET_DBUS_PATH,
                            QDBusConnection::sessionBus());

        setData(I18N_NOOP(name), I18N_NOOP("error"), false);
        setData(I18N_NOOP(name), I18N_NOOP("transfers"),
                                kget_interface.transfers().value());
    } else {
        setData(I18N_NOOP(name), I18N_NOOP("error"), true);
        setData(I18N_NOOP(name), I18N_NOOP("errorMessage"),
                                I18N_NOOP("Is KGet up and running?"));
    }
}

bool KGetEngine::isDBusServiceRegistered()
{
    return interface->isServiceRegistered(KGET_DBUS_SERVICE);
}

#include "kgetengine.moc"
