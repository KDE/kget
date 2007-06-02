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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "bttransfer.h"
#include "btthread.h"

#include <sigc++/bind.h>
#include <torrent/torrent.h>
#include <torrent/bencode.h>
#include <torrent/rate.h>

#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>

#include <QDomElement>
#include <QFile>

BTTransfer::BTTransfer(TransferGroup* parent, TransferFactory* factory,
               Scheduler* scheduler, const KUrl& src, const KUrl& dest,
               const QDomElement * e)
  : Transfer(parent, factory, scheduler, src, dest, e),
    m_chunksTotal(0), m_chunksDownloaded(0),
    m_peersConnected(0), m_peersNotConnected(0)
{
    BTThread::initialize(); //check: is this thread active always?? :-O
    kDebug(5001) << "new bt transfer" << endl;

    //We already know that the file is local (the check is in the Factory)
    QFile file(src.path());
    kDebug(5001) << "Opening file: " << src.path() << endl;

    if(file.open(QIODevice::ReadOnly))
    {
        kDebug(5001) << "***********Bittorrent file opened" << endl;
        QByteArray data = file.readAll();
        kDebug(5001) << "Stream of size: " << data.size() << endl; 
        kDebug(5001) << "Stream of data: " << endl << data.data()  << endl;
        bencodeStream.write(data.data(), data.size());

        file.close();
    }
    else
    {
        kDebug(5001) << "***********Unable to open bittorrent file!" << endl;
        kDebug(5001) << file.errorString() << endl;
    }

    connect(&timer, SIGNAL(timeout()), SLOT(update()));
}

BTTransfer::~BTTransfer()
{
    BTThread::stop();
}

bool BTTransfer::isResumable() const
{
    return true;
}

int BTTransfer::chunksTotal()
{
    if (download.is_valid() && download.is_active())
        return download.get_chunks_total();
    else
        return -1;
}

int BTTransfer::chunksDownloaded()
{
    if (download.is_valid() && download.is_active())
        return download.get_chunks_done();
    else
        return -1;
}

int BTTransfer::peersConnected()
{
    if (download.is_valid() && download.is_active())
        return download.get_peers_connected();
    else
        return -1;

}

int BTTransfer::peersNotConnected()
{
    if (download.is_valid() && download.is_active())
        return download.get_peers_not_connected();
    else
        return -1;
}

void BTTransfer::start()
{
    startTime = QTime::currentTime();
    setStatus(Job::Running, i18n("Analizing torrent.."), SmallIcon("xmag"));
    setTransferChange(Tc_Status, true);

    resume();
}

void BTTransfer::stop()
{
    kDebug(5001) << endl << "bt stopped" << endl << endl;
    timer.stop();
    if (download.is_valid()) 
    {
        download.stop();
//          download.hash_save();
        m_speed = 0;
        setStatus(Job::Stopped, i18n("Stopped"), SmallIcon("process-stop"));
        setTransferChange(Tc_Speed | Tc_Status, true);

        startTime = QTime();
    }
}

int BTTransfer::elapsedTime() const
{
    return startTime.secsTo(QTime::currentTime());
}

int BTTransfer::remainingTime() const
{
    // we should use the average rate here
    int rate = speed();
    return (rate <= 0) ? -1 :
        (int)((totalSize() - processedSize()) / rate);
}

void BTTransfer::resume()
{
    kDebug(5001) << endl << "resume dl" << endl << endl;
    if (!download.is_valid())
    {
        try
        {
            download = torrent::download_add(&bencodeStream);
            // deallocate stream
            bencodeStream.str(std::string());
            // set directory
            download.set_root_dir(std::string(qPrintable(dest().directory(false))));

            if (download.get_entry_size() == 1) 
            {
                //set filename too?
            }

            //       trackerSucceeded = download.signal_tracker_succeded
            // 	(sigc::bind(sigc::mem_fun(*this, &BTTransfer::trackerMessage), 
            // 		    "succeeded"));
            //       trackerFailed = download.signal_tracker_failed
            // 	(sigc::mem_fun(*this, &BTTransfer::trackerMessage));

            downloadDone = download.signal_download_done
            (sigc::mem_fun(*this, &BTTransfer::downloadFinished));

            hashingDone = download.signal_hash_done
            (sigc::mem_fun(*this, &BTTransfer::hashingFinished));
        }
        catch (std::exception& e)
        {
            // line below only compiles with exceptions activated
            // kDebug(5001) << "exception " << e.what() << endl;
            return;
        }
        kDebug(5001) << "still alive" << endl;
    }

    if(!download.is_active())
    {
        if (!download.is_open()) 
        {
            kDebug(5001) << endl << "second turn" << endl << endl;
            download.open();
        }
        if (!download.is_hash_checked()) 
        {
            download.hash_check(false);
            return;
        }
        try 
        {
            kDebug(5001) << endl << "third turn" << endl << endl;
            setStatus(status(), i18n("Connecting.."), SmallIcon("connect-creating"));
            setTransferChange(Tc_Status, true);

            download.start();
        }
        catch (std::exception& e)
        {
        // the line below only compiles with exceptions activated
        // kDebug(5001) << "Resume exception " << e.what() << endl << endl;
        }
        timer.start(1 * 1000);
        return;
    }
}

void BTTransfer::remove()
{
    kDebug(5001) << endl << "bt removed" << endl << endl;
    timer.stop();
    if (download.is_valid() && download.is_active()) 
    {
        download.stop();
        download.close();
        torrent::download_remove(download.get_hash());
    }
}

void BTTransfer::trackerMessage(std::string msg)
{
    kDebug(5001) << "trackerMessage" << endl;
    kDebug(5001) << msg.c_str() << endl;
}

void BTTransfer::downloadFinished()
{
    kDebug(5001) << "bt transfer done " << endl;
    setStatus(Job::Finished, i18n("Finished"), SmallIcon("ok"));
    setTransferChange(Tc_Status, true);
}

void BTTransfer::hashingFinished()
{
    kDebug(5001) << "hashing finished " << endl;

    setTransferChange(Tc_Status, true);
    resume();
}

void BTTransfer::update()
{
    kDebug(5001) << "update" << endl;
    if (!download.is_valid() || !download.is_active()) 
    {
        kDebug(5001) << "timer running on invalid or inactive download" << endl;
        timer.stop();
        return;
    }
  
//   kDebug(5001) << "dl name " << download.get_name().c_str() << endl;
//   kDebug(5001) << "processedSize " << download.get_bytes_done() << endl;
//   kDebug(5001) << "rate down " << download.get_rate_down() << endl;
//   kDebug(5001) << "rate up " << download.get_rate_up() << endl;
//   kDebug(5001) << "bytes up " << download.get_bytes_up() << endl;
//   kDebug(5001) << "bytes down " << download.get_bytes_down() << endl;
//   kDebug(5001) << "chunk size " << download.get_chunks_size() << endl;
//   kDebug(5001) << "chunks done " << download.get_chunks_done() << endl;
//   kDebug(5001) << "chunks total " << download.get_chunks_total() << endl;
//   kDebug(5001) << "peers conn " << download.get_peers_connected() << endl;
//   kDebug(5001) << "handshakes " << torrent::get(torrent::HANDSHAKES_TOTAL) << endl;

    BTThread::lock();
    m_totalSize = download.get_bytes_total();
    m_processedSize = download.get_bytes_done();
    m_speed = download.get_read_rate().rate();
    BTThread::unlock();

    //Make sure we are really downloading the torrent before setting the status
    //text to "Downloading.."
    if( m_speed &&
       (statusText() != i18n("Downloading..")) &&
       (status() != Job::Finished) )
    {
        setStatus(status(), i18n("Downloading.."), SmallIcon("media-playback-start"));
        setTransferChange(Tc_Status);
    }

    if (m_totalSize > 0) 
    {
        m_percent = (int)((100.0 * m_processedSize) / m_totalSize);
        setTransferChange(Tc_Percent);
    }

    if (m_chunksTotal != chunksTotal())
    {
        m_chunksTotal = chunksTotal();
        setTransferChange(Tc_ChunksTotal);
    }

    if (m_chunksDownloaded != chunksDownloaded())
    {
        m_chunksDownloaded = chunksDownloaded();
        setTransferChange(Tc_ChunksDownloaded);
    }

    if (m_peersConnected != peersConnected())
    {
        m_peersConnected = peersConnected();
        setTransferChange(Tc_PeersConnected);
    }

    if (m_peersNotConnected != peersNotConnected())
    {
        m_peersNotConnected = peersNotConnected();
        setTransferChange(Tc_PeersNotConnected);
    }

    setTransferChange(Tc_ProcessedSize | Tc_Speed | Tc_TotalSize, true);
}

void BTTransfer::save(QDomElement e) // krazy:exclude=passbyvalue
{
    Transfer::save(e);

    if (download.is_valid() && !download.is_active()) 
    {
        QDomDocument doc(e.ownerDocument());
        QDomElement bencode(doc.createElement("bencode"));
        e.appendChild(bencode);
        std::stringstream s;
        s << download.get_bencode();
        bencode.appendChild(doc.createTextNode(s.str().c_str()));
    }
}

void BTTransfer::load(const QDomElement &e)
{
    if (!e.isNull())
    {
        Transfer::load(e);

        QDomElement first(e.firstChild().toElement());
        if (!first.isNull() &&  (first.tagName() == "bencode") )
        {
            bencodeStream << qPrintable(first.text());
        }
    }
}

#include "bttransfer.moc"
