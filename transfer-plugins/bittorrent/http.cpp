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

#include "http.h"
#include "http.moc"

#include <torrent/exceptions.h>
#include <iostream>

#include <kio/job.h>
#include <kio/scheduler.h>
#include <kdebug.h>

void signalDone(torrent::Http* http);
void signalFailed(torrent::Http* http, const QString& err);

Http::Http()
  : job(0)
{
}

void Http::start()
{
  if (job) {
    // throw torrent::internal_error("Tried to start already running http job");
    kDebug(5001) << "Tried to start already running http job";
  }
  job = KIO::get(KUrl(get_url().c_str()), KIO::NoReload, KIO::HideProgressInfo);
  connect(job, SIGNAL(data(KIO::Job*, const QByteArray&)),
	  SLOT(data(KIO::Job*, const QByteArray&)));
  connect(job, SIGNAL(result(KIO::Job*)), SLOT(result(KIO::Job*)));
}

void Http::close()
{
  if (job) {
    KIO::Scheduler::cancelJob(job);
    job = 0;
  }
}

void Http::data(KIO::Job* job, const QByteArray& data)
{
  Q_UNUSED(job);
  m_stream->write(data.data(), data.size());
}

void Http::result(KIO::Job* job)
{
  if (job->error()) {
    signalFailed(this, job->errorString());
  }
  else {
    signalDone(this);
  }
  this->job = 0;
}

Http* Http::createObject()
{
  // hope they don't leak in the client code
  return new Http();
}

Http::SlotFactory Http::getFactory()
{
  return sigc::ptr_fun(&Http::createObject);
}

#ifdef emit
#undef emit
void signalDone(torrent::Http* http)
{
  http->signal_done().emit();
}

void signalFailed(torrent::Http* http, const QString& err)
{
  Q_UNUSED(err)
  http->signal_failed().emit("http error");
}
#endif
