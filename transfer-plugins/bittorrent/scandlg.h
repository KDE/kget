/***************************************************************************
 *   Copyright (C) 2007 by Joris Guisson and Ivan Vasic                    *
 *   joris.guisson@gmail.com                                               *
 *   ivasic@gmail.com                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#ifndef KT_SCANDLG_HH
#define KT_SCANDLG_HH

#include <QDialog>
#include <QMutex>
#include <QTimer>

#include "ui_scandlg.h"
#include <torrent/job.h>
#include <version.h>

namespace bt
{
class TorrentInterface;
}

namespace kt
{
class TorrentInterface;

class ScanDlg : public QDialog
{
    Q_OBJECT
public:
    ScanDlg(KJob *job, QWidget *parent);
    ~ScanDlg() override;

protected:
    /// Handle the close event
    void closeEvent(QCloseEvent *e) override;

protected Q_SLOTS:
    void reject() override;
    void accept() override;

private Q_SLOTS:
    void description(KJob *job, const QString &title, const QPair<QString, QString> &field1, const QPair<QString, QString> &field2);
    void result(KJob *job);
    void percent(KJob *job, unsigned long percent);

private:
    bt::Job *m_job;
    QProgressBar *m_progress;
    QPushButton *m_cancel;
    QLabel *m_torrent_label;
    QLabel *m_chunks_failed;
    QLabel *m_chunks_found;
    QLabel *m_chunks_not_downloaded;
    QLabel *m_chunks_downloaded;
};
}

#endif
