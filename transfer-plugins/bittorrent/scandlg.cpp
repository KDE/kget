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
#include "scandlg.h"
#include <interfaces/torrentinterface.h>
#include <util/error.h>
#include <util/log.h>

#include <KIO/Global>
#include <KLocalizedString>
#include <KMessageBox>
#include <KStandardGuiItem>

#include <KConfigGroup>
#include <QCloseEvent>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

using namespace bt;

namespace kt
{
ScanDlg::ScanDlg(KJob *job, QWidget *parent)
    : QDialog(parent)
    , m_job(static_cast<Job *>(job))
{
    Ui::ScanDlgBase ui;
    auto *widget = new QWidget(this);
    auto *mainLayout = new QVBoxLayout;
    ui.setupUi(widget);
    mainLayout->addWidget(widget);
    m_torrent_label = ui.torrent_label;
    m_chunks_found = ui.chunks_found;
    m_chunks_failed = ui.chunks_failed;
    m_chunks_downloaded = ui.chunks_downloaded;
    m_chunks_not_downloaded = ui.chunks_not_downloaded;
    m_progress = ui.progress;
    m_cancel = ui.cancel;
    KGuiItem::assign(m_cancel, KStandardGuiItem::cancel());
    connect(m_cancel, &QPushButton::clicked, this, &ScanDlg::reject);
    m_progress->setMaximum(100);
    m_progress->setValue(0);
    connect(m_job, &KJob::description, this, &ScanDlg::description);
    connect(m_job, &KJob::result, this, &ScanDlg::result);
    connect(m_job, &KJob::percentChanged, this, &ScanDlg::percent);
}
ScanDlg::~ScanDlg()
{
}

void ScanDlg::closeEvent(QCloseEvent *)
{
    if (m_job) {
        m_job->kill(false);
        m_job = nullptr;
    } else
        accept();
}

void ScanDlg::reject()
{
    if (m_job) {
        m_job->kill(false);
        m_job = nullptr;
    }
    QDialog::reject();
    deleteLater();
}

void ScanDlg::accept()
{
    QDialog::accept();
    deleteLater();
}

void ScanDlg::description(KJob *job, const QString &title, const QPair<QString, QString> &field1, const QPair<QString, QString> &field2)
{
    Q_UNUSED(job)
    Q_UNUSED(title)
    m_chunks_found->setText(field1.first);
    m_chunks_failed->setText(field1.second);
    m_chunks_downloaded->setText(field1.first);
    m_chunks_not_downloaded->setText(field2.second);
}

void ScanDlg::result(KJob *job)
{
    if (job->error() && job->error() != KIO::ERR_USER_CANCELED) {
        KMessageBox::error(nullptr, i18n("Error scanning data: %1", job->errorString()));
    }
    m_job = nullptr;
    m_progress->setValue(100);
    disconnect(m_cancel, &QPushButton::clicked, this, &ScanDlg::reject);
    connect(m_cancel, &QPushButton::clicked, this, &ScanDlg::accept);
}

void ScanDlg::percent(KJob *job, unsigned long percent)
{
    Q_UNUSED(job)
    m_progress->setValue(percent);
}
}

#include "moc_scandlg.cpp"
