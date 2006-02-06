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

#ifndef HTTP_H
#define HTTP_H

#include <torrent/http.h>

#include <QObject>
#include <QPointer>

namespace KIO {
  class Job;
  class TransferJob;
}

class Http : public QObject, public torrent::Http
{
  Q_OBJECT

public:
  Http();
  void start();
  void close();

  typedef sigc::slot0<Http*>  SlotFactory;
  static SlotFactory getFactory();
  static Http* createObject();


private slots:
  void data(KIO::Job* job, const QByteArray& data);
  void result(KIO::Job* job);

private:
  QPointer<KIO::TransferJob> job;
};

#endif
