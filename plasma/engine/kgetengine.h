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

#ifndef KGETENGINE_H
#define KGETENGINE_H

#include <plasma/dataengine.h>

#include <QtDBus/QDBusConnectionInterface>

class OrgKdeKgetMainInterface;

class KGetEngine : public Plasma::DataEngine
{
    Q_OBJECT

    public:
        KGetEngine(QObject* parent, const QVariantList& args);
        ~KGetEngine();

        void init();

    protected:
        bool sourceRequestEvent(const QString &name);
        bool updateSourceEvent(const QString& source);

    private slots:
        void getKGetData();
        void updateData();
        void slotTransfersAdded(const QStringList &urls, const QStringList &dBusObjectPaths);
        void slotTransfersRemoved(const QStringList &urls, const QStringList &dBusObjectPaths);

    private:
        bool isDBusServiceRegistered();

    private:
        QDBusConnectionInterface *interface;
        OrgKdeKgetMainInterface *m_kget;
        static const quint16 MINIMUM_UPDATE_INTERVAL;
        static const QString KGET_DBUS_SERVICE;
        static const QString KGET_DBUS_PATH;
};

K_EXPORT_PLASMA_DATAENGINE(kget, KGetEngine)

#endif
