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
#include "kgetdbus.h"

#include <QtDBus/QDBusConnectionInterface>
#include <QTimer>
#include <KDebug>

#include "plasma/datacontainer.h"

KGetEngine::KGetEngine(QObject* parent, const QVariantList& args)
    : Plasma::DataEngine(parent)
{
    Q_UNUSED(args);

	interface = QDBusConnection::sessionBus().interface();
    setMinimumUpdateInterval(MINIMUM_UPDATE_INTERVAL);
}

KGetEngine::~KGetEngine()
{
}

void KGetEngine::setRefreshTime(uint time)
{
    setUpdateInterval(time);
}

uint KGetEngine::refreshTime() const
{
    return 1000;
}

bool KGetEngine::sourceRequested(const QString &name)
{
    updateSource(name);
    return true;
}

bool KGetEngine::updateSource(const QString &name)
{
    if(QString::compare(name, "KGet") == 0) {
        getKGetData(name);
    }
    return true;
}

void KGetEngine::getKGetData(const QString &name)
{
	clearData(name);

	if(isDBusServiceRegistered()) {
		OrgKdeKgetInterface kget_interface(KGET_DBUS_SERVICE, KGET_DBUS_PATH,
							QDBusConnection::sessionBus());

		setData(I18N_NOOP(name), I18N_NOOP("error"), false);
		setData(I18N_NOOP(name), I18N_NOOP("transfers"),
						kget_interface.transfers().value());
	}
	else {
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
