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

MmsThread::MmsThread(const QString& url, const QString& name, int begin, int end)
: QThread(),
    m_sourceUrl(url),
    m_fileName(name),
    m_begin(begin),
    m_end(end)
{}

void MmsThread::run()
{
    m_file.open(qstrdup(m_fileName.toAscii()));
    m_mms = mmsx_connect(NULL, NULL, qstrdup(m_sourceUrl.toAscii()) , 1e6);
    m_file.seekp(m_begin);
    mmsx_seek(0, m_mms, m_begin, 0);

    while (m_begin < m_end) {
        if ((m_begin + 1024) > m_end) { 
            const int var = m_end - m_begin;
            char data[var];
            mmsx_read(0, m_mms, data, var);
            m_locker.lock();
            m_file.write(data, var);
            emit reading(var);
            m_locker.unlock();
            m_begin = m_end;
            quit(); // NOTE: Keep "quit()" here, if not then the thread never finish.
        } else {
            char data[1024];
            mmsx_read(0, m_mms, data, 1024);
            m_locker.lock();
            m_file.write(data, 1024);
            emit reading(1024);
            m_locker.unlock();
            m_begin += 1024;
        }
    }
    m_file.close();
    mmsx_close(m_mms);
    exec();
}

void MmsThread::stop()
{
    m_begin = m_end; //TODO: Find another way to stop the threads!!
}
