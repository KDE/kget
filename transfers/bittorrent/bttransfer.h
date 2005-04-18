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
#include <qobject.h>
#include "core/transfer.h"

#include <sstream>

#include <qtimer.h>
#include <qdatetime.h>

namespace KIO {
  class Job;
}

class BTTransfer : public QObject, public Transfer
{
  Q_OBJECT

public:
  BTTransfer(TransferGroup* parent, TransferFactory* factory,
	     Scheduler* scheduler, const KURL& src, const KURL& dest);
  BTTransfer(TransferGroup* parent, TransferFactory* factory, 
	     Scheduler* scheduler, QDomNode* node);
  ~BTTransfer();


  unsigned long totalSize() const;
  unsigned long processedSize() const;
  int percent() const;
  int speed() const;
  void start();
  void stop();
  int elapsedTime() const;
  int remainingTime() const;
  bool isResumable() const;

public slots:
  bool slotResume();
  void slotStop();
  void slotRemove();
    
protected:
  void read(QDomNode* node);
  void write(QDomNode* node);
  
private slots:
  void data(KIO::Job* job, const QByteArray& data);
  void result(KIO::Job* job);
  void update();

private:
  QTime startTime;
  QTimer timer;
  std::stringstream bencodeStream;
  torrent::Download download;

  void trackerMessage(std::string msg);
  void downloadFinished();
  void hashingFinished();

  
  sigc::connection trackerSucceeded;
  sigc::connection trackerFailed;
  sigc::connection downloadDone;
  sigc::connection hashingDone;

};

#endif
