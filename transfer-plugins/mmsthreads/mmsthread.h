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

#ifndef MMSTHREAD_HPP
#define MMSTHREAD_HPP
#include <libmms/mmsx.h>
#include <QtCore/QThread>
#include <QtCore/QFile>
#include <QtCore/QMutex>

using namespace std;

class MmsThread : public QThread
{
    Q_OBJECT
    public:
        MmsThread(const QString& url, const QString& name, int begin, int end);
        void run();
        void stop();

    private:
        QString m_sourceUrl;
        QString m_fileName;
        int m_begin;
        int m_end;
        QMutex m_locker;
        bool m_download;

    signals:
        void signReading(int data, int m_end, int m_begin);
        void signIsConnected(bool connected);
};

#endif // MMSTHREAD_HPP
