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

#ifndef BTTRANSFER_H
#define BTTRANSFER_H

#include <torrent/torrent.h>
#include <sstream>

#include <qobject.h>
#include <qtimer.h>
#include <qdatetime.h>

#include "core/transfer.h"


namespace KIO
{
    class Job;
}

class BTTransfer : public QObject, public Transfer
{
    Q_OBJECT

    public:
        BTTransfer(TransferGroup* parent, TransferFactory* factory,
                    Scheduler* scheduler, const KURL& src, const KURL& dest,
                    const QDomElement * e = 0 );
        ~BTTransfer();

        //Job virtual functions
        void start();
        void stop();
        int elapsedTime() const;
        int remainingTime() const;
        bool isResumable() const;

        void save(QDomElement e);

    protected:
        void load(QDomElement e);

    private slots:
        void update();

    private:
        void resume();
        void remove(); //Now I put this functions here. Shouldn't it
                       //be integrated in the destructor?
        void trackerMessage(std::string msg);
        void downloadFinished();
        void hashingFinished();

        QTime startTime;
        QTimer timer;
        std::stringstream bencodeStream;
        torrent::Download download;

        sigc::connection trackerSucceeded;
        sigc::connection trackerFailed;
        sigc::connection downloadDone;
        sigc::connection hashingDone;
};

#endif
