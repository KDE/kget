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

#include "mmsthread.h"
#include <vector>

MmsThread::MmsThread(const QString &url, const QString &name, int begin, int end)
    : QThread()
    , m_sourceUrl(url)
    , m_fileName(name)
    , m_begin(begin)
    , m_end(end)
    , m_download(true)
{
}

void MmsThread::run()
{
    /** Seems that some times mmsx_read not read well.*/
    int readed;
    mmsx_t *mms;
    QFile file(m_fileName);
    /** Opening the file for write the information.*/
    file.open(QIODevice::ReadWrite);
    file.seek(m_begin);

    /** Connecting to the url*/
    mms = mmsx_connect(NULL, NULL, qstrdup(m_sourceUrl.toLatin1()), 1e6);
    if (mms) {
        m_locker.lock();
        Q_EMIT signIsConnected(true);
        m_locker.unlock();
        /** If the connections result successful it start the download.*/
        mmsx_seek(nullptr, mms, m_begin, 0);
        while ((m_begin < m_end) && m_download) {
            if ((m_begin + 1024) > m_end) {
                const int var = m_end - m_begin;
                std::vector<char> data(var);
                readed = mmsx_read(nullptr, mms, data.data(), var);
                m_locker.lock();
                Q_EMIT signReading(var, m_end, m_begin = m_end);
                /** Writing the readed to the file */
                if (readed) {
                    file.write(data.data(), readed);
                }
                m_locker.unlock();
            } else {
                std::vector<char> data(1024);
                readed = mmsx_read(nullptr, mms, data.data(), 1024);
                m_locker.lock();
                Q_EMIT signReading(1024, m_end, m_begin += 1024);
                /** Writing the readed to the file */
                if (readed) {
                    file.write(data.data(), readed);
                }
                m_locker.unlock();
            }
        }
        file.close();
        mmsx_close(mms);
        quit(); // NOTE: Keep "quit()" here, if not then the thread never Q_EMIT the signal finish.
    } else {
        /** If the connections not result successfully then stop all the download*/
        m_locker.lock();
        Q_EMIT signIsConnected(false);
        m_locker.unlock();
        quit(); // NOTE: Keep "quit()" here, if not then the thread never Q_EMIT the signal finish.
    }
    exec();
}

void MmsThread::stop()
{
    m_download = false;
}

#include "moc_mmsthread.cpp"
