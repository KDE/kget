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

#include "mmstransfer.h"

#include "core/kget.h"
#include <KIO/DeleteJob>
#include <kmessagebox.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kdebug.h>

#include <QtCore/QFile>



MmsTransfer::MmsTransfer(TransferGroup * parent, TransferFactory * factory,
                        Scheduler * scheduler, const KUrl & source, const
                        KUrl &dest, const QDomElement * e)
    : Transfer(parent, factory, scheduler, source, dest, e),
    m_mmsdownload(0)
{
    kDebug(5001) << "Mms transfer initialized: " + m_source.prettyUrl();
}

MmsTransfer::~MmsTransfer()
{
    if (m_mmsdownload) {
        m_mmsdownload->quit();
        m_mmsdownload->deleteLater();
    }
}

void MmsTransfer::start()
{
    kDebug(5001) << "Trying to start Mms-transfer: " + m_source.prettyUrl();
    
    if (status() == Running) {
        return;
    }
    
    setStatus(Job::Running, i18nc("transfer state: running", "Running...."),
                SmallIcon("network-connect"));
    m_mmsdownload = new MmsDownload(m_source.prettyUrl(), m_dest.pathOrUrl(),
                                    MmsSettings::threads());
    connect(m_mmsdownload, SIGNAL(signThreadFinish()), this, SLOT(slotResult()));
    connect(m_mmsdownload, SIGNAL(signBrokenUrl()), this, SLOT(slotBrokenUrl()));
    connect(m_mmsdownload, SIGNAL(signNotAllowMultiDownload()), this,
            SLOT(slotNotAllowMultiDownload()));
    connect(m_mmsdownload, SIGNAL(signTotalSize(qulonglong)), this,
            SLOT(slotTotalSize(qulonglong)));
    connect(m_mmsdownload, SIGNAL(signDownloaded(qulonglong)), this,
            SLOT(slotProcessedSizeAndPercent(qulonglong)));
    connect(m_mmsdownload, SIGNAL(signSpeed(unsigned long)), this,
            SLOT(slotSpeed(unsigned long)));
    m_mmsdownload->start();
    setTransferChange(Tc_Status, true);
}

void MmsTransfer::stop()
{
    if ((status() == Stopped) || (status() == Finished)) {
        return;
    }
    // NOTE: When all threads stop, m_mmsdownload will be delete in MmsTransfer::slotResult.
    if (m_mmsdownload->threadsAlive() > 0) {
        m_mmsdownload->stopTransfer();
    }

    setStatus(Job::Stopped, i18nc("transfer state: stopped", "Stopped"), SmallIcon("process-stop"));
    m_downloadedSize = 0;
    m_percent = 0;
    m_downloadSpeed = 0;
    setTransferChange(Tc_Percent | Tc_DownloadedSize | Tc_DownloadSpeed, true);
}

void MmsTransfer::deinit()
{
    if (status() != Job::Finished) {
        KIO::Job *del = KIO::del(m_dest.path(), KIO::HideProgressInfo);
        KIO::NetAccess::synchronousRun(del, 0);
    }
}

void MmsTransfer::slotResult()
{
    if (m_downloadedSize == m_totalSize && m_totalSize != 0) {
        setStatus(Job::Finished, i18nc("Transfer State:Finished","Finished"),
                   SmallIcon("dialog-ok"));
        m_percent = 100;
        m_downloadSpeed = 0;
        setTransferChange(Tc_Percent | Tc_DownloadSpeed, true);
    }

    if (status() == Stopped) {
        m_downloadedSize = 0;
        m_percent = 0;
        m_downloadSpeed = 0;
        setTransferChange(Tc_Percent | Tc_DownloadedSize | Tc_DownloadSpeed, true);
    }

    if (m_mmsdownload->threadsAlive() == 0) {
        m_mmsdownload->quit();
        m_mmsdownload->deleteLater();
        m_mmsdownload = NULL;
    }
}

void MmsTransfer::slotTotalSize(qulonglong size)
{
    m_totalSize = size;
    setTransferChange(Tc_TotalSize, true);
}

void MmsTransfer::slotSpeed(ulong speed)
{
    m_downloadSpeed = speed;
    setTransferChange(Tc_DownloadSpeed, true);
}

void MmsTransfer::slotProcessedSizeAndPercent(qulonglong size)
{
    m_downloadedSize = size;
    m_percent = (m_downloadedSize * 100) / m_totalSize;
    setTransferChange(Tc_DownloadedSize | Tc_Percent, true);
}

void MmsTransfer::slotBrokenUrl()
{
    //FIXME: It continue trying of download and never stop, if you stop the download it show
    //you the error.
    setError(i18n("Download failed, could not access this URL."), SmallIcon("dialog-cancel"),
            Job::NotSolveable);
    setTransferChange(Tc_Status);
}

void MmsTransfer::slotNotAllowMultiDownload()
{
    KGet::showNotification(0, "notification",
                           i18n("This URL does not allow multiple connections,\n"
                                "the download will take longer."));
}

#include "mmstransfer.moc"
