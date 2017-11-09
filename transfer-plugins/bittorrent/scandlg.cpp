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
#include <QCloseEvent>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandardguiitem.h>
#include <util/error.h>
#include <util/log.h>
#include <interfaces/torrentinterface.h>
using namespace bt;

namespace kt
{
	ScanDlg::ScanDlg(KJob *job, QWidget* parent)
    : KDialog(parent), m_job(static_cast<Job*>(job))
    {
        setButtons(KDialog::None);
        Ui::ScanDlgBase ui;
        QWidget *widget = new QWidget(this);
        ui.setupUi(widget);
        setMainWidget(widget);
        m_torrent_label = ui.torrent_label;
        m_chunks_found = ui.chunks_found;
        m_chunks_failed = ui.chunks_failed;
        m_chunks_downloaded = ui.chunks_downloaded;
        m_chunks_not_downloaded = ui.chunks_not_downloaded;
        m_progress = ui.progress;
        m_cancel = ui.cancel;
        KGuiItem::assign(m_cancel, KStandardGuiItem::cancel());
        connect(m_cancel,SIGNAL(clicked()),this,SLOT(reject()));
        m_progress->setMaximum(100);
        m_progress->setValue(0);
        connect(m_job, SIGNAL(description(KJob*,QString,QPair<QString,QString>,QPair<QString,QString>)),
                       SLOT(description(KJob*,QString,QPair<QString,QString>,QPair<QString,QString>)));
        connect(m_job, SIGNAL(result(KJob*)),
                       SLOT(result(KJob*)));
        connect(m_job, SIGNAL(percent(KJob*,ulong)),
                       SLOT(percent(KJob*,ulong)));
    }
    ScanDlg::~ScanDlg()
    {
    }
     
	void ScanDlg::closeEvent(QCloseEvent* )
	{
        if (m_job) {
            m_job->kill(false);
            m_job = nullptr;
        }
		else
		        accept();
	}

	void ScanDlg::reject()
	{
		if (m_job) {
			m_job->kill(false);
            m_job = nullptr;
        }
		KDialog::reject();
		deleteLater();
	}

	void ScanDlg::accept()
	{
		KDialog::accept();
		deleteLater();
	}

	void ScanDlg::description(KJob *job, const QString &title, const QPair<QString, QString > &field1, const QPair< QString, QString > &field2)
	{
		m_chunks_found->setText(field1.first);
		m_chunks_failed->setText(field1.second);
		m_chunks_downloaded->setText(field1.first);
		m_chunks_not_downloaded->setText(field2.second);
	}
	
	void ScanDlg::result(KJob *job)
	{
        if (job->error() && job->error() != KIO::ERR_USER_CANCELED) {
            KMessageBox::error(nullptr,i18n("Error scanning data: %1",job->errorString()));
        }
	    m_job = nullptr;
        m_progress->setValue(100);
        disconnect(m_cancel,SIGNAL(clicked()),this,SLOT(reject()));
        connect(m_cancel,SIGNAL(clicked()),this,SLOT(accept()));
	}
	
	void ScanDlg::percent(KJob *job, unsigned long percent)
	{
	        m_progress->setValue(percent);
	}
}



