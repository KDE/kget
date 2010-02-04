/* This file is part of the KDE project

   Copyright (C) 2009 Lukas Appelhans <l.appelhans@gmx.de>
   Copyright (C) 2009 Matthias Fuchs <mat69@gmx.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#ifndef DBUSTRANSFERWRAPPER_H
#define DBUSTRANSFERWRAPPER_H

#include "core/transferhandler.h"

#include <QDBusVariant>

class TransferHandler;

class DBusTransferWrapper : public QObject
{
    Q_OBJECT
    public:
        DBusTransferWrapper(TransferHandler * parent);
        ~DBusTransferWrapper();

    public slots:
        int capabilities() const;
        void start();
        void stop();
        int status() const;
        int elapsedTime() const;
        int remainingTime() const;

        /**
         * @return the transfer's group's name
         */
        QString groupName() const;

        /**
         * @return the source url
         */
        QString source() const;

        /**
         * @return the dest url
         */
        QString dest() const;

        /**
         * Move the download to the new destination
         * @param newDirectory is a directory where the download should be stored
         * @returns true if newDestination can be used
         */
        bool setDirectory(const QString &directory);

        /**
         * @return the total size of the transfer in bytes
         */
        qulonglong totalSize() const;

        /**
         * @return the downloaded size of the transfer in bytes
         */
        qulonglong downloadedSize() const;

        /**
         * @return the uploaded size of the transfer in bytes
         */
        qulonglong uploadedSize() const;

        /**
         * @return the progress percentage of the transfer
         */
        int percent() const;

        /**
         * @return the download speed of the transfer in bytes/sec
         */
        int downloadSpeed() const;

        /**
         * @return the upload speed of the transfer in bytes/sec
         */
        int uploadSpeed() const;

        /**
         * Set an UploadLimit for the transfer
         * @note this UploadLimit is not visible in the GUI
         * @param ulLimit upload Limit
         */
        void setUploadLimit(int ulLimit, int limit);

        /**
         * Set a DownloadLimit for the transfer
         * @note this DownloadLimit is not visible in the GUI
         * @param dlLimit download Limit
         */
        void setDownloadLimit(int dlLimit, int limit);

        /**
         * @return the upload Limit of the transfer in KiB
         */
        int uploadLimit(int limit) const;

        /**
         * @return the download Limit of the transfer in KiB
         */
        int downloadLimit(int limit) const;

        /**
         * Set the maximum share-ratio
         * @param ratio the new maximum share-ratio
         */
        void setMaximumShareRatio(double ratio);

        /**
         * @return the maximum share-ratio
         */
        double maximumShareRatio();

        /**
         * @return a string describing the current transfer status
         */
        QString statusText() const;

        /**
         * @return a pixmap associated with the current transfer status
         */
        QDBusVariant statusPixmap() const;

        /**
         * Returns the dBusObjectPath to the verifier
         * @param file for wich to return the verifier
         */
        QString verifier(const QString &file);

        /**
         * Tries to repair file
         * @param file the file of a download that should be repaired,
         * if not defined all files of a download are going to be repaird
         * @return true if a repair started, false if it was not nescessary
         */
        bool repair(const QString &file);

    signals:
        /**
         * Emitted when the transfer changes
         */
        void transferChangedEvent(int transferChange);

        /**
         * Emitted whe the capabilities of the transfer changes
         */
        void capabilitiesChanged();

    private slots:
        void slotTransferChanged(TransferHandler *transfer, TransferHandler::ChangesFlags changeflags);

    private:
        TransferHandler *m_transfer;
};

#endif
