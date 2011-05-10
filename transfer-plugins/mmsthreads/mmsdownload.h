/*
    This file is part of the KDE project
    Copyright (C) 2011 Ernesto Rodriguez Ortiz <eortiz@uci.cu>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef MMSDOWNLOAD_H
#define MMSDOWNLOAD_H

#include <QString>
#include <QTimer>
#include <libmms/mmsx.h>
#include <fstream>

#include "mmsthread.h"


class MmsDownload : public QThread
{
    Q_OBJECT
    public:
        MmsDownload(const QString& url, const QString& name, int amountsThread);
        ~MmsDownload();
        void run();
        void stopTransfer();
        int threadsAlive();

    public slots:
        void slotThreadFinish();
        void slotRead(int reading);
        void slotSpeedChanged();

    signals:
        void signBrokenUrl();
        void signNotAllowMultiDownload();
        void signThreadFinish();
        qulonglong signDownloaded(qulonglong reading);
        qulonglong signTotalSize(qulonglong size);
        unsigned long signSpeed(unsigned long bytes_per_second);

    private:
        bool isWorkingUrl();
        void startTransfer();

        QString m_sourceUrl;
        QString m_fileName;
        int m_amountsThread;
        qulonglong m_downloadedSize;
        qulonglong m_prevDownloadedSizes;
        mmsx_t* m_mms;
        QTimer* m_speedTimer;
        QList<MmsThread*> m_threadList;

};

#endif // MMSDOWNLOAD_H
