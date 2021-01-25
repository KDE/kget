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

#ifndef MMSTRANSFER_H
#define MMSTRANSFER_H


#include <QTime>
#include <KIO/DeleteJob>
#include "core/kget.h"
#include "mmsdownload.h"
#include "mmssettings.h"
#include "core/transfer.h"

class MmsTransfer : public Transfer
{
    Q_OBJECT

    public:
        MmsTransfer(TransferGroup * parent, TransferFactory * factory,
                    Scheduler * scheduler, const QUrl & src, const QUrl & dest,
                    const QDomElement * e = nullptr);
        ~MmsTransfer() override;

    public Q_SLOTS:
        void start() override;
        void stop() override;
        void deinit(Transfer::DeleteOptions options) override;

    private Q_SLOTS:
        void slotResult();
        void slotTotalSize(qulonglong size);
        void slotProcessedSizeAndPercent(qulonglong size);
        void slotSpeed(ulong bytes_per_sec);
        void slotNotAllowMultiDownload();
        void slotBrokenUrl();
        void slotConnectionsErrors(int connections);

    private:
        MmsDownload* m_mmsdownload;
        int m_amountThreads;
        bool m_retryDownload;
        QString m_fileTemp;
};

#endif
