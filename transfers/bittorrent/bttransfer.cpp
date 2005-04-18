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

#include "bttransfer.h"
#include "bttransfer.moc"

#include <torrent/bencode.h>

#include "btthread.h"

#include <sigc++/bind.h>

#include <kio/job.h>
#include <kio/scheduler.h>
#include <kdebug.h>
#include <klocale.h>

#include <qdom.h>

BTTransfer::BTTransfer(TransferGroup* parent, TransferFactory* factory,
		       Scheduler* scheduler, const KURL& src, const KURL& dest)
  : Transfer(parent, factory, scheduler, src, dest)
{
  BTThread::initialize();
  kdDebug() << "new bt transfer" << endl;
  slotResume();

  connect(&timer, SIGNAL(timeout()), SLOT(update()));
}
 
BTTransfer::BTTransfer(TransferGroup* parent, TransferFactory* factory,
		       Scheduler* scheduler, QDomNode* node)
  : Transfer(parent, factory, scheduler, node)
{
  slotResume();
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

unsigned long BTTransfer::totalSize() const
{
  return m_totalSize;
}

unsigned long BTTransfer::processedSize() const
{
  return m_processedSize;
}

int BTTransfer::percent() const
{
  return m_percent;
}

void BTTransfer::start()
{
  startTime = QTime::currentTime();
  slotResume();
}

void BTTransfer::stop()
{
  slotStop();
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

int BTTransfer::speed() const
{
  return m_speed;
}

bool BTTransfer::slotResume()
{
  kdDebug() << endl << "resume dl" << endl << endl;
  if (!download.is_valid()) {
    kdDebug() << endl << "fresh start" << endl << endl;
    KIO::TransferJob* job = KIO::get(source(), false, false);
    connect(job, SIGNAL(data(KIO::Job*, const QByteArray&)),
	    SLOT(data(KIO::Job*, const QByteArray&)));
    connect(job, SIGNAL(result(KIO::Job*)), SLOT(result(KIO::Job*)));
    KIO::Scheduler::scheduleJob(job);
    return true;
  }
  if(!download.is_active()) {
    if (!download.is_open()) {
      kdDebug() << endl << "second turn" << endl << endl;
      download.open();
    }
    if (!download.is_hash_checked()) {
      download.hash_check(false);
      return true;
    }
    try {
      kdDebug() << endl << "third turn" << endl << endl;
      download.start();
    }
    catch (std::exception& e) {
      // hte line below only compiles with exceptions activated
      // kdDebug() << "slotResume exception " << e.what() << endl << endl;
    }
    m_statusText = i18n("Running");
    setTransferChange(Tc_Status, true);
    timer.start(1 * 1000);
    return true;
  }
  return false;
}
 
void BTTransfer::slotStop()
{
  kdDebug() << endl << "bt stopped" << endl << endl;
  timer.stop();
  if (download.is_valid()) {
    download.stop();
    download.hash_save();
    m_statusText = i18n("Stopeed");
    setTransferChange(Tc_Status, true);
    startTime = QTime();
  }
}
 
void BTTransfer::slotRemove()
{
  kdDebug() << endl << "bt removed" << endl << endl;
  timer.stop();
  if (download.is_valid() && download.is_active()) {
    download.stop();
    download.close();
    torrent::download_remove(download.get_hash());
  }
}
    
void BTTransfer::data(KIO::Job* job, const QByteArray& data)
{
  Q_UNUSED(job);
  bencodeStream.write(data.data(), data.size());
}
 
void BTTransfer::result(KIO::Job* job)
{
  kdDebug() << "finished tracker download" << endl;
  if (job->error()) {
    // handle error
  }
  else {
    kdDebug() << "no error so far " << endl;
    
    try {
      download = torrent::download_create(&bencodeStream);
      // deallocate stream
      bencodeStream.str(std::string());
      // set directory
      download.set_root_dir(std::string(dest().directory(false).ascii()));

      if (download.get_entry_size() == 1) {
	// set filename too?
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
	
      slotResume();
    }
    catch (std::exception& e) {
      // line below only compiles with exceptions activated
      // kdDebug() << "exception " << e.what() << endl;
      return;
    }
    kdDebug() << "still alive" << endl;
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
  setTransferChange(Tc_Status);
}

void BTTransfer::hashingFinished()
{
  kdDebug() << "hashing finished " << endl;
  
  m_statusText = i18n("Trying");
  setTransferChange(Tc_Status);
  slotResume();
}

void BTTransfer::update()
{
  kdDebug() << "update" << endl;
  if (!download.is_valid() || !download.is_active()) {
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

  BTThread::lock();
  m_totalSize = download.get_bytes_total();
  m_processedSize = download.get_bytes_done();
  m_speed = download.get_rate_down();
  BTThread::unlock();
  if (m_totalSize > 0) {
    m_percent = (int)((100.0 * m_processedSize) / m_totalSize);
    setTransferChange(Tc_Percent);
  }
  setTransferChange(Tc_Status);
  setTransferChange(Tc_ProcessedSize);
  setTransferChange(Tc_Speed);
  setTransferChange(Tc_TotalSize, true);
}

void BTTransfer::read(QDomNode* node)
{
  Transfer::read(node);
  QDomElement e(node->toElement());
  if (!e.isNull()) {
    QDomElement first(e.firstChild().toElement());
    if (!first.isNull()) {
      if (first.tagName() == "bencode") {
	bencodeStream << first.text().ascii();
      }
    }
  }
}

void BTTransfer::write(QDomNode* node)
{
  Transfer::write(node);
  if (download.is_valid() && !download.is_active()) {
    QDomDocument doc(node->ownerDocument());
    QDomElement bencode(doc.createElement("bencode"));
    node->appendChild(bencode);
    std::stringstream s;
    s << torrent::download_bencode(download.get_hash());
    bencode.appendChild(doc.createTextNode(s.str().c_str()));
  }
}
