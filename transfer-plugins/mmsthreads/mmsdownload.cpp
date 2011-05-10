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

MmsDownload::MmsDownload(const QString &url, const QString &name, int amountsThread)
: QThread(),
  m_sourceUrl(url),
  m_fileName(name),
  m_amountsThread(amountsThread),
  m_downloadedSize(0),
  m_prevDownloadedSizes(0),
  m_mms(NULL)
{
    m_speedTimer = new QTimer(this);
    m_speedTimer->setInterval(1000);
    connect(m_speedTimer, SIGNAL(timeout()), this, SLOT(slotSpeedChanged()));
}

MmsDownload::~MmsDownload()
{
    if (m_mms) {
        mmsx_close(m_mms);
    }
}

void MmsDownload::run()
{
    if (isWorkingUrl()) {
        startTransfer();
    } else {
        emit signBrokenUrl();
        exit(0); // NOTE: Keep this here for finish the thread.
    }
    exec();
}


bool MmsDownload::isWorkingUrl()
{
    m_mms = mmsx_connect(NULL, NULL, qstrdup(m_sourceUrl.toAscii()), 1e9);
    return m_mms;
}

void MmsDownload::startTransfer()
{
    m_amountsThread = mmsx_get_seekable(m_mms) ? m_amountsThread : 1;
    if (m_amountsThread == 1) {
        emit signNotAllowMultiDownload();
    }

    const qulonglong total = mmsx_get_length(m_mms);
    emit signTotalSize(total);
    int part = mmsx_get_length(m_mms) / m_amountsThread;
    int sum = 0;
    m_speedTimer->start();
    for (int i = 0; i < m_amountsThread; i++) {
        if (i + 1 == m_amountsThread) {
            part = total - sum;
        }

        MmsThread* thread = new MmsThread(m_sourceUrl, m_fileName, sum, sum+part);
        sum +=part;
        m_threadList.append(thread);
        connect(thread, SIGNAL(finished()), this, SLOT(slotThreadFinish()));
        connect(thread, SIGNAL(reading(int)), this, SLOT(slotRead(int)));
        thread->start();
    }
}

void MmsDownload::slotSpeedChanged()
{
    emit signSpeed(m_downloadedSize - m_prevDownloadedSizes);
    m_prevDownloadedSizes = m_downloadedSize;
}


void MmsDownload::stopTransfer()
{
    //NOTE: Here only is called thread->stop() because when the thread finish it emit a signal
    // and slotThreadFinish(); is called where the thread is delete calling deleteLater(); and
    // m_threadList is cleaning using removeAll().
    foreach (MmsThread* thread, m_threadList) {
        thread->stop();
        thread->quit();
    }
}

int MmsDownload::threadsAlive()
{
    return m_threadList.length();
}


void MmsDownload::slotThreadFinish()
{
    foreach (MmsThread *thread, m_threadList) {
        if (thread->isFinished()) {
            m_threadList.removeAll(thread);
            thread->deleteLater();
        }
    }
    if (m_threadList.isEmpty()) {
        emit signThreadFinish();
    }
}

void MmsDownload::slotRead(int reading)
{
    m_downloadedSize += reading;
    emit signDownloaded(m_downloadedSize);
}