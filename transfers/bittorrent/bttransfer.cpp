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



#include <sigc++/bind.h>
#include <torrent/torrent.h>
#include <torrent/bencode.h>

#include <qdom.h>
#include <qfile.h>

#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>

#include "bttransfer.h"
#include "btthread.h"

BTTransfer::BTTransfer(TransferGroup* parent, TransferFactory* factory,
               Scheduler* scheduler, const KURL& src, const KURL& dest,
               const QDomElement * e)
  : Transfer(parent, factory, scheduler, src, dest, e)
{
    BTThread::initialize(); //check: is this thread active always?? :-O
    kdDebug() << "new bt transfer" << endl;

    //We already know that the file is local (the check is in the Factory)
    QFile file(src.path());
    kdDebug() << "Opening file: " << src.path() << endl;

    if(file.open(IO_ReadOnly))
    {
        kdDebug() << "***********Bittorrent file opened" << endl;
        QByteArray data = file.readAll();
        kdDebug() << "Stream of size: " << data.size() << endl; 
        kdDebug() << "Stream of data: " << endl << data.data()  << endl;
        bencodeStream.write(data.data(), data.size());

        file.close();
    }
    else
    {
        kdDebug() << "***********Unable to open bittorrent file!" << endl;
        kdDebug() << file.errorString() << endl;
    }

    connect(&timer, SIGNAL(timeout()), SLOT(update()));

    m_statusText = i18n("Stopped");
    m_statusPixmap = SmallIcon("stop");
}

BTTransfer::~BTTransfer()
{
    BTThread::stop();
}

bool BTTransfer::isResumable() const
{
    return true;
}

void BTTransfer::start()
{
    startTime = QTime::currentTime();
    setStatus(Job::Running);
    resume();
}

void BTTransfer::stop()
{
    kdDebug() << endl << "bt stopped" << endl << endl;
    timer.stop();
    if (download.is_valid()) 
    {
        download.stop();
        download.hash_save();
        m_statusText = i18n("Stopped");
        m_statusPixmap = SmallIcon("stop");
        setTransferChange(Tc_Status, true);
        setStatus(Job::Stopped);

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
    kdDebug() << endl << "resume dl" << endl << endl;
    if (!download.is_valid())
    {
        try
        {
            download = torrent::download_create(&bencodeStream);
            // deallocate stream
            bencodeStream.str(std::string());
            // set directory
            download.set_root_dir(std::string(dest().directory(false).ascii()));

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
            // kdDebug() << "exception " << e.what() << endl;
            return;
        }
        kdDebug() << "still alive" << endl;
    }

    if(!download.is_active())
    {
        if (!download.is_open()) 
        {
            kdDebug() << endl << "second turn" << endl << endl;
            download.open();
        }
        if (!download.is_hash_checked()) 
        {
            download.hash_check(false);
            return;
        }
        try 
        {
            kdDebug() << endl << "third turn" << endl << endl;
            download.start();
        }
        catch (std::exception& e)
        {
        // the line below only compiles with exceptions activated
        // kdDebug() << "Resume exception " << e.what() << endl << endl;
        }
        m_statusText = i18n("Connecting..");
        m_statusPixmap = SmallIcon("connect_creating");
        setTransferChange(Tc_Status, true);
        timer.start(1 * 1000);
        return;
    }
}

void BTTransfer::remove()
{
    kdDebug() << endl << "bt removed" << endl << endl;
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
    kdDebug() << "trackerMessage" << endl;
    kdDebug() << msg.c_str() << endl;
}

void BTTransfer::downloadFinished()
{
    kdDebug() << "bt transfer done " << endl;
    m_statusText = i18n("Finished");
    m_statusPixmap = SmallIcon("ok");
    setTransferChange(Tc_Status, true);
}

void BTTransfer::hashingFinished()
{
    kdDebug() << "hashing finished " << endl;

    setTransferChange(Tc_Status, true);
    resume();
}

void BTTransfer::update()
{
    kdDebug() << "update" << endl;
    if (!download.is_valid() || !download.is_active()) 
    {
        kdDebug() << "timer running on invalid or inactive download" << endl;
        timer.stop();
        return;
    }
  
//   kdDebug() << "dl name " << download.get_name().c_str() << endl;
//   kdDebug() << "processedSize " << download.get_bytes_done() << endl;
//   kdDebug() << "rate down " << download.get_rate_down() << endl;
//   kdDebug() << "rate up " << download.get_rate_up() << endl;
//   kdDebug() << "bytes up " << download.get_bytes_up() << endl;
//   kdDebug() << "bytes down " << download.get_bytes_down() << endl;
//   kdDebug() << "chunk size " << download.get_chunks_size() << endl;
//   kdDebug() << "chunks done " << download.get_chunks_done() << endl;
//   kdDebug() << "chunks total " << download.get_chunks_total() << endl;
//   kdDebug() << "peers conn " << download.get_peers_connected() << endl;
//   kdDebug() << "handshakes " << torrent::get(torrent::HANDSHAKES_TOTAL) << endl;

    if(m_statusText != i18n("Running"))
    {
        m_statusText = i18n("Downloading..");
        m_statusPixmap = SmallIcon("tool_resume");
        setTransferChange(Tc_Status, true);
    }

    BTThread::lock();
    m_totalSize = download.get_bytes_total();
    m_processedSize = download.get_bytes_done();
    m_speed = download.get_rate_down();
    BTThread::unlock();
    if (m_totalSize > 0) 
    {
        m_percent = (int)((100.0 * m_processedSize) / m_totalSize);
        setTransferChange(Tc_Percent);
    }
    setTransferChange(Tc_Status);
    setTransferChange(Tc_ProcessedSize);
    setTransferChange(Tc_Speed);
    setTransferChange(Tc_TotalSize, true);
}

void BTTransfer::save(QDomElement e)
{
    if (download.is_valid() && !download.is_active()) 
    {
        QDomDocument doc(e.ownerDocument());
        QDomElement bencode(doc.createElement("bencode"));
        e.appendChild(bencode);
        std::stringstream s;
        s << torrent::download_bencode(download.get_hash());
        bencode.appendChild(doc.createTextNode(s.str().c_str()));
    }
}

void BTTransfer::load(QDomElement e)
{
    Transfer::load(e);

    if (!e.isNull())
    {
        QDomElement first(e.firstChild().toElement());
        if (!first.isNull() &&  (first.tagName() == "bencode") )
        {
            bencodeStream << first.text().ascii();
        }
    }
}

#include "bttransfer.moc"
