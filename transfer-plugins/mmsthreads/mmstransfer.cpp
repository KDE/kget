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

MmsTransfer::MmsTransfer(TransferGroup * parent, TransferFactory * factory,
                        Scheduler * scheduler, const KUrl & source, const
                        KUrl &dest, const QDomElement * e)
    : Transfer(parent, factory, scheduler, source, dest, e),
    m_mmsdownload(NULL),
    m_amountThreads(MmsSettings::threads()),
    m_retryDownload(false)
{
    m_fileTemp = KStandardDirs::locateLocal("appdata", m_dest.fileName());
    kDebug(5001) << "Mms transfer initialized: " + m_source.prettyUrl();
}

MmsTransfer::~MmsTransfer()
{
    /** If m_mmsdownload is not deleted we delete it before end.*/
    if (m_mmsdownload) {
        m_mmsdownload->quit();
        m_mmsdownload->deleteLater();
    }
}

void MmsTransfer::start()
{
    /** Starting the download, is created the thread m_mmsdownload and is started the download*/
    if (m_mmsdownload || status() == Finished) {
        return;
    }

    setStatus(Job::Running, i18nc("transfer state: running", "Running...."),
              SmallIcon("media-playback-start"));
    m_mmsdownload = new MmsDownload(m_source.prettyUrl(), m_dest.pathOrUrl(),
                                    m_fileTemp, m_amountThreads);
    connect(m_mmsdownload, SIGNAL(finished()), this, SLOT(slotResult()));
    connect(m_mmsdownload, SIGNAL(signBrokenUrl()), this, SLOT(slotBrokenUrl()));
    connect(m_mmsdownload, SIGNAL(signNotAllowMultiDownload()), this,
            SLOT(slotNotAllowMultiDownload()));
    connect(m_mmsdownload, SIGNAL(signTotalSize(qulonglong)), this,
            SLOT(slotTotalSize(qulonglong)));
    connect(m_mmsdownload, SIGNAL(signDownloaded(qulonglong)), this,
            SLOT(slotProcessedSizeAndPercent(qulonglong)));
    connect(m_mmsdownload, SIGNAL(signSpeed(ulong)), this,
            SLOT(slotSpeed(ulong)));
    connect(m_mmsdownload, SIGNAL(signRestartDownload(int)), this,
            SLOT(slotConnectionsErrors(int)));
    m_mmsdownload->start();
    setTransferChange(Tc_Status, true);
}

void MmsTransfer::stop()
{
    /** The download is stopped, we call m_mmsdownload->stopTransfer() and when all threads
     * are finish m_mmsdownload will be deleted in MmsTransfer::slotResult().
     */
    if ((status() == Stopped) || (status() == Finished)) {
        return;
    }
    
    if (m_mmsdownload) {
        if (m_mmsdownload->threadsAlive() > 0) {
            m_mmsdownload->stopTransfer();
        }
    }

    setStatus(Job::Stopped, i18nc("transfer state: stopped", "Stopped"),
                SmallIcon("process-stop"));
    m_downloadSpeed = 0;
    setTransferChange(Tc_Status | Tc_DownloadSpeed, true);
}

void MmsTransfer::deinit(Transfer::DeleteOptions options)
{
    /** Deleting the temporary file and the unfinish file*/
    if (options & Transfer::DeleteFiles) {
        KIO::Job *del = KIO::del(m_fileTemp, KIO::HideProgressInfo);
        KIO::NetAccess::synchronousRun(del, 0);
        del = KIO::del(m_dest.path(), KIO::HideProgressInfo);
        KIO::NetAccess::synchronousRun(del, 0);
    }
}

void MmsTransfer::slotResult()
{   
    /** This slot is connected with the signal finish of m_mmsdownload*/
    /** Deleting m_mmsdownload.*/
    m_mmsdownload->deleteLater();
    m_mmsdownload = NULL;
    
    /** If the download end without problems is changed the status to Finished and is deleted
     * the temporary file where is saved the status of all threads that download the file.     
     */
    if (m_downloadedSize == m_totalSize && m_totalSize != 0) {
        setStatus(Job::Finished, i18nc("Transfer State:Finished","Finished"),
                   SmallIcon("dialog-ok"));
        m_percent = 100;
        m_downloadSpeed = 0;
        setTransferChange(Tc_Status | Tc_Percent | Tc_DownloadSpeed, true);
        KIO::Job *del = KIO::del(m_fileTemp, KIO::HideProgressInfo);
        KIO::NetAccess::synchronousRun(del, 0);
    }
     
    /** If m_retryDownload == true then some threads has fail to connect, so the download was
     * stopped in MmsTransfer::slotConnectionsErrors() and here when all the connected thread 
     * are finished we delete the temporary file and we start again the download using the amount
     * of threads defined in MmsTransfer::slotConnectionsErrors().
     */
    if (m_retryDownload) {
        m_retryDownload = false;
        KIO::Job *del = KIO::del(m_fileTemp, KIO::HideProgressInfo);
        KIO::NetAccess::synchronousRun(del, 0);
        start();
    }
}

void MmsTransfer::slotTotalSize(qulonglong size)
{
    m_totalSize = size;
    setTransferChange(Tc_TotalSize, true);
}

void MmsTransfer::slotSpeed(ulong speed)
{
    m_downloadSpeed = (status() == Running) ? speed : 0;
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
    setError(i18n("Download failed, could not access this URL."), SmallIcon("dialog-cancel"),
            Job::NotSolveable);
    setTransferChange(Tc_Status, true);
}

void MmsTransfer::slotNotAllowMultiDownload()
{
    /** Some stream not allow seek in to a position, so we can't use more than one thread to 
     * download the file, this is notify to the user because the download will take longer.
     */
    KGet::showNotification(0, "notification", i18n("This URL does not allow multiple connections,\n"
                                "the download will take longer."));
}

void MmsTransfer::slotConnectionsErrors(int connections)
{
    /** Here is called stop() for stop the download, set a new amount of thread
     * and set m_retryDownload = true for restart the download when mmsdownload is finish and 
     * emit a singal connected with MmsTransfer::slotResult(), see in MmsTransfer::slotResult()
     * for understand when its started again the download.
     */
    stop();
    m_retryDownload = true;
    if (connections) {
        m_amountThreads = connections;
    } else {
        m_amountThreads--;
    }
}

#include "mmstransfer.moc"
