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

#include <QTimer>
#include <QDateTime>

#include "core/transfer.h"


namespace KIO
{
    class Job;
}

class BTTransfer : public QObject, public Transfer
{
    Q_OBJECT

    public:
        /**
         * Here we define the Bittorrent specific flags.
         */
        enum BTTransferChange
        {
            Tc_ChunksTotal       = 0x00010000,
            Tc_ChunksDownloaded  = 0x00020000,
            Tc_PeersConnected    = 0x00040000,
            Tc_PeersNotConnected = 0x00080000
        };

        BTTransfer(TransferGroup* parent, TransferFactory* factory,
                    Scheduler* scheduler, const KUrl& src, const KUrl& dest,
                    const QDomElement * e = 0 );
        ~BTTransfer();

        //Job virtual functions
        void start();
        void stop();
        int elapsedTime() const;
        int remainingTime() const;
        bool isResumable() const;

        //Bittorrent specific functions
        int chunksTotal();
        int chunksDownloaded();
        int peersConnected();
        int peersNotConnected();

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

        int m_chunksTotal;
        int m_chunksDownloaded;
        int m_peersConnected;
        int m_peersNotConnected;

        sigc::connection trackerSucceeded;
        sigc::connection trackerFailed;
        sigc::connection downloadDone;
        sigc::connection hashingDone;
};

#endif
