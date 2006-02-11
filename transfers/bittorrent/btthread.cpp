/*
 *  Copyright (C) 2005 Felix Berger <felixberger@beldesign.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <torrent/torrent.h>
#include "btthread.h"
#include "http.h"

#include <kdebug.h>

BTThread* BTThread::instance = 0;
int BTThread::initialized = 0;

void BTThread::initialize()
{
    if (!initialized++)
    {
        torrent::Http::set_factory(Http::getFactory());
        torrent::initialize();
        torrent::listen_open(6890, 6999);
        instance = new BTThread();
        instance->start();
    }
    kDebug() << "initialized1 = " << initialized << endl;
}

void BTThread::stop()
{
    kDebug() << "initialized2 = " << initialized << endl;
    if (!--initialized)
    {
        //I've disabled this line because we can't delete the instance
        //if we have a QMutexLocker. Felix, is this mutex necessary?
        //(removing it solves some crashes when deleting bittorrent transfers)
        //QMutexLocker locker(&instance->mutex);
        instance->terminate();
        instance->wait();
        delete instance;
        instance = 0;
        torrent::cleanup();
    }
}

BTThread::~BTThread()
{
}

void BTThread::lock()
{
    if (instance) 
    {
        instance->mutex.lock();
    }
}

void BTThread::unlock()
{
    if (instance) 
    {
        instance->mutex.unlock();
    }
}

void BTThread::run()
{
    while (true) 
    {
        int max_fd = 0;
        fd_set rd, wr, er;
        FD_ZERO (&rd);
        FD_ZERO (&wr);
        FD_ZERO (&er);

        torrent::mark(&rd, &wr, &er, &max_fd);

        uint64_t t = torrent::get_next_timeout();
	// TODO check if values make sense and we sleep enough on select
        if (t > 1000000)
            t = 1000000;

        timeval timeout = {t / 1000000, t % 1000000};

        max_fd = select(max_fd + 1, &rd, &wr, &er, &timeout);

        if (max_fd >= 0) 
        {
            QMutexLocker locker(&mutex);
            torrent::work(&rd, &wr, &er, max_fd);
        }
    }
}
