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

#include "mmsdownload.h"

const int SPEEDTIMER = 1000;//1 second...

MmsDownload::MmsDownload(const QString &url, const QString &name, const QString &temp, 
                         int amountsThread)
: QThread(),
  m_sourceUrl(url),
  m_fileName(name),
  m_fileTemp(temp),
  m_amountThreads(amountsThread),
  m_connectionsFails(0),
  m_connectionsSuccefully(0),
  m_downloadedSize(0),
  m_mms(NULL)
{
    m_speedTimer = new QTimer(this);
    m_speedTimer->setInterval(SPEEDTIMER);
    connect(m_speedTimer, SIGNAL(timeout()), this, SLOT(slotSpeedChanged()));
}

MmsDownload::~MmsDownload()
{
    if (m_mms) {
        mmsx_close(m_mms);
    }
    m_speedTimer->stop();
    m_speedTimer->deleteLater();
}

void MmsDownload::run()
{
    if (isWorkingUrl()) {
        splitTransfer();
        startTransfer();
    } else {
        emit signBrokenUrl();
        quit();
    }
    exec();
}


bool MmsDownload::isWorkingUrl()
{
    /** Check if the URL is working, if it can't connect then not start the download.*/
    m_mms = mmsx_connect(NULL, NULL, qstrdup(m_sourceUrl.toAscii()), 1e9);
    return m_mms;
}

void MmsDownload::splitTransfer()
{
    /** We split the download in similar and each part is asigned to a thread and thi is saved in
     * a map named m_mapEndIni. If we resume the download, then the temporal file will exist
     * and we dont have to split the download only use it.
     */
    m_amountThreads = mmsx_get_seekable(m_mms) ? m_amountThreads : 0;
    if (m_amountThreads == 0) {
        m_amountThreads = 1;
        emit signNotAllowMultiDownload();
        QFile::remove(m_fileTemp);
    }

    const qulonglong total = mmsx_get_length(m_mms);
    emit signTotalSize(total);
    
    if (QFile::exists(m_fileTemp)) {
        unSerialization();
    } else {
        int part = mmsx_get_length(m_mms) / m_amountThreads;
        int ini = 0;
        int end = 0;
        for (int i = 0; i < m_amountThreads; i++) {
            if (i + 1 == m_amountThreads) {
                part = total - ini;
            }
            end = ini + part;
            m_mapEndIni.insert(end, ini);
            ini += part;
        }
    }
}

void MmsDownload::startTransfer()
{   
    m_speedTimer->start();
    QMap<int, int>::const_iterator iterator = m_mapEndIni.constBegin();
    while (iterator != m_mapEndIni.constEnd()) {
        MmsThread* thread = new MmsThread(m_sourceUrl, m_fileName,
                                          iterator.value(), iterator.key());
        m_threadList.append(thread);
        connect(thread, SIGNAL(finished()), this, SLOT(slotThreadFinish()));
        connect(thread, SIGNAL(signIsConnected(bool)), this, SLOT(slotIsThreadConnected(bool)));
        connect(thread, SIGNAL(signReading(int,int,int)), this, SLOT(slotRead(int,int,int)));
        thread->start();
        ++iterator;
    }
}

void MmsDownload::slotSpeedChanged()
{
    /** Using the same speed calculating datasourcefactory uses (use all downloaded data 
     * of the last 10 secs)
     */
    qulonglong speed;
    if (m_prevDownloadedSizes.size()) {
        speed = (m_downloadedSize - m_prevDownloadedSizes.first()) / (SPEEDTIMER *
            m_prevDownloadedSizes.size() / 1000);//downloaded in 1 second
    } else {
        speed = 0;
    }
    
    m_prevDownloadedSizes.append(m_downloadedSize);
    if(m_prevDownloadedSizes.size() > 10)
        m_prevDownloadedSizes.removeFirst();
    
    emit signSpeed(speed);
    serialization();
}


void MmsDownload::stopTransfer()
{   
    /** Here only is called thread->stop() because when the thread finish it emit a signal
     * and slotThreadFinish(); is called where the thread is delete calling deleteLater(); and
     * m_threadList is cleaning using removeAll().
     */
    foreach (MmsThread* thread, m_threadList) {
        thread->stop();
        thread->quit();
    }
}

int MmsDownload::threadsAlive()
{
    return m_threadList.size();
}


void MmsDownload::slotThreadFinish()
{
    MmsThread* thread = qobject_cast<MmsThread*>(QObject::sender());
    m_threadList.removeAll(thread);
    thread->deleteLater();

    if (m_threadList.isEmpty()) {
        serialization();
        quit();
    }
}

void MmsDownload::slotRead(int reading, int thread_end, int thread_in)
{
    /** We update the status of the thread in the map and emit a signal for update the download
     * speed.
     */
    if (thread_in == thread_end) {
        m_mapEndIni.remove(thread_end);    
    } else {
        m_mapEndIni[thread_end] = thread_in;
    }
    m_downloadedSize += reading;
    emit signDownloaded(m_downloadedSize); 
}

void MmsDownload::slotIsThreadConnected(bool connected)
{
    /** All thread emit a signal connected with this slot, if they get connected succefully
     * the value of "connected" will be true, and will be false if they can't connected. When all
     * the thread emited the signal the amount of m_connectionsSuccefully and m_connectionsFails
     * will be equal to m_amountThreads and we emit a signal to restart the download in
     * mmstransfer using the amount of connections succefully connected.
     */
    if (connected) {
        m_connectionsSuccefully++;
    } else {
        m_connectionsFails++;
    }
    if ((m_connectionsFails != 0) && 
        (m_connectionsFails + m_connectionsSuccefully == m_amountThreads)) {
        emit signRestartDownload(m_connectionsSuccefully);
    }
}

void MmsDownload::serialization()
{
    /** Here we save the status of the download to the temporal file for resume the download 
     * if we stop it.
     */
    QFile file(m_fileTemp);
    file.open(QIODevice::WriteOnly);
    QDataStream out(&file);
    out << m_mapEndIni << m_downloadedSize << m_prevDownloadedSizes;
    file.close();
}

void MmsDownload::unSerialization()
{
    /** Here we read the status of the download to the temporal file for resume the download
     */
    QFile file(m_fileTemp);
    file.open(QIODevice::ReadOnly);
    QDataStream in(&file);
    in >> m_mapEndIni >> m_downloadedSize >> m_prevDownloadedSizes;
    file.close();
}
