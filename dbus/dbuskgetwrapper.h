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

#ifndef DBUSKGETWRAPPER
#define DBUSKGETWRAPPER

#include <QObject>
#include <QVariantMap>

class MainWindow;
class TransferHandler;

class DBusKGetWrapper : public QObject
{
    Q_OBJECT

    public:
        DBusKGetWrapper(MainWindow *parent);
        ~DBusKGetWrapper();

        QStringList addTransfer(const QString& src, const QString& destDir = QString(), bool start = false);
        bool delTransfer(const QString& dbusObjectPath);
        void showNewTransferDialog(const QStringList &urls);
        bool dropTargetVisible() const;
        void setDropTargetVisible(bool setVisible);
        void setOfflineMode(bool online);
        bool offlineMode() const;
        QVariantMap transfers() const;
        int transfersSpeed() const;
        void importLinks(const QList <QString> &links);
        bool isSupported(const QString &url) const;

    signals:
        void transferAddedRemoved();
        void transfersAdded(const QStringList &urls, const QStringList &dBusObjectPaths);
        void transfersRemoved(const QStringList &urls, const QStringList &dbusObjectPaths);

    private slots:
        void slotTransfersAdded(const QList<TransferHandler*> &transfers);
        void slotTransfersRemoved(const QList<TransferHandler*> &transfers);

    private:
        MainWindow *m_mainWindow;
        QHash<TransferHandler*, QPair<QString, QString> > m_transfers;
};

#endif
