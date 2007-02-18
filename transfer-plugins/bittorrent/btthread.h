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

#ifndef BTTHREAD_H
#define BTTHREAD_H

#include <QThread>
#include <QMutex>

/**
 * Singleton thread for the bittorrent library. Waits in a select for the
 * bittorrent file descriptors and executes the work on the descriptors in its
 * run method.
 *
 * The work part is protected by a private mutex which can be locked through
 * the static public inerfaces lock() and unlock(). Code in different
 * threads (the Qt eventloop thread) should lock before accessing values of a
 * torrent::download object.
 *
 * It also does all the library initialization and cleanup calls.
 */
class BTThread : public QThread
{
    private:
        BTThread()
        {
        }
        ~BTThread();

    public:
        /**
        * Initialize torrent library the instance thread and start it.
        *
        * torrent::initialize() should be already called before.
        */
        static void initialize();
        static void stop();
        static void lock();
        static void unlock();

        void run();

    private:
        static BTThread* instance;
        static int initialized;
        QMutex mutex;
};

#endif
