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

#ifndef KGETENGINE_H
#define KGETENGINE_H

#include <plasma/dataengine.h>

#include <QtDBus/QDBusConnectionInterface>

static const uint MINIMUM_UPDATE_INTERVAL = 1000;
static const QString KGET_DBUS_SERVICE = "org.kde.kget";
static const QString KGET_DBUS_PATH = "/KGet";


class KGetEngine : public Plasma::DataEngine
{
    Q_OBJECT
    Q_PROPERTY(uint refreshTime READ refreshTime WRITE setRefreshTime)

public:
    KGetEngine(QObject* parent, const QVariantList& args);
    ~KGetEngine();

    QStringList sources() const;

    void setRefreshTime(uint time);
    uint refreshTime() const;

protected:
    bool sourceRequestEvent(const QString &name);
    bool updateSourceEvent(const QString& source);

private slots:
    void getKGetData(const QString &name);

private:
    bool isDBusServiceRegistered();

    QDBusConnectionInterface *interface;
};

K_EXPORT_PLASMA_DATAENGINE(kget, KGetEngine)

#endif
