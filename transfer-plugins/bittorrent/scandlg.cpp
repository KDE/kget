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
#if LIBKTORRENT_VERSION >= 0x010100
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
        m_cancel->setGuiItem(KStandardGuiItem::cancel());
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
            m_job = 0;
        }
		else
		        accept();
	}

	void ScanDlg::reject()
	{
		if (m_job) {
			m_job->kill(false);
            m_job = 0;
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
            KMessageBox::error(0,i18n("Error scanning data: %1",job->errorString()));
        }
	    m_job = 0;
        m_progress->setValue(100);
        disconnect(m_cancel,SIGNAL(clicked()),this,SLOT(reject()));
        connect(m_cancel,SIGNAL(clicked()),this,SLOT(accept()));
	}
	
	void ScanDlg::percent(KJob *job, unsigned long percent)
	{
	        m_progress->setValue(percent);
	}
#else
	ScanDlg::ScanDlg(QWidget* parent) 
		: KDialog(parent),bt::DataCheckerListener(false),mutex(QMutex::Recursive)
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
		m_cancel->setGuiItem(KStandardGuiItem::cancel());
		connect(m_cancel,SIGNAL(clicked()),this,SLOT(onCancelPressed()));
		connect(&timer,SIGNAL(timeout()),this,SLOT(update()));
		tc = 0;
		silently = false;
		restart = false;
		scanning = false;
		num_chunks = 0;
		total_chunks = 0;
		num_downloaded = 0;
		num_failed = 0;
		num_found = 0;
		num_not_downloaded = 0;
		m_progress->setMaximum(100);
		m_progress->setValue(0);
	}
	
	ScanDlg::~ScanDlg()
	{
	}

	void ScanDlg::scan()
	{
		try
		{
			tc->startDataCheck(this);
			timer.start(500);
			scanning = true;
		}
		catch (bt::Error & err)
		{
			KMessageBox::error(0,i18n("Error scanning data: %1",err.toString()));
		}
	}

	void ScanDlg::execute(bt::TorrentInterface* tc,bool silently)
	{
		m_torrent_label->setText(i18n("Scanning data of <b>%1</b> :",tc->getStats().torrent_name));
		adjustSize();
		m_cancel->setEnabled(true);
		this->silently = silently;
		this->tc = tc;
		num_chunks = 0;
		total_chunks = 0;
		num_downloaded = 0;
		num_failed = 0;
		num_found = 0;
		num_not_downloaded = 0;
		if (auto_import || tc->getStats().running)
			restart = true;

		qm_priority = tc->getPriority();

		if (tc->getStats().running)
		{
			tc->stop();
		}
		
		scan();
	}

	void ScanDlg::progress(bt::Uint32 num,bt::Uint32 total)
	{
		QMutexLocker lock(&mutex);
		num_chunks = num;
		total_chunks = total;
	}
		 
	void ScanDlg::status(bt::Uint32 failed,Uint32 found,Uint32 downloaded,Uint32 not_downloaded)
	{
		QMutexLocker lock(&mutex);
		num_failed = failed;
		num_found = found;
		num_downloaded = downloaded;
		num_not_downloaded = not_downloaded;
	}
		
	void ScanDlg::finished()
	{
		QMutexLocker lock(&mutex);
		scanning = false;
		timer.stop();
		progress(100,100);
		update();
		if (!isStopped())
		{
			if (restart)
			{
				tc->start();
			}
			
			if (silently)
				accept();
			else
			{
				// cancel now becomes a close button
				m_cancel->setGuiItem(KStandardGuiItem::close()); 
				disconnect(m_cancel,SIGNAL(clicked()),this,SLOT(onCancelPressed()));
				connect(m_cancel,SIGNAL(clicked()),this,SLOT(accept()));
			}
		}
		else
		{
			if (restart)
			{
				tc->start();
			}
			
			KDialog::reject();
		}
	}
	
	void ScanDlg::error(const QString &message)
    {
        KMessageBox::error(0,i18n("Error scanning data: %1",message));
    }
		 
	void ScanDlg::closeEvent(QCloseEvent* )
	{
		if (scanning)
			stop();
		else
		        accept();
	}

	void ScanDlg::reject()
	{
		if (scanning)
			stop();
		else
		{
			KDialog::reject();
			deleteLater();
		}
	}

	void ScanDlg::accept()
	{
		KDialog::accept();
		deleteLater();
	}

	void ScanDlg::onCancelPressed()
	{
		stop();
	}

	void ScanDlg::update()
	{
		QMutexLocker lock(&mutex);
		m_progress->setMaximum(total_chunks);
		m_progress->setValue(num_chunks);
		m_chunks_found->setText(QString::number(num_downloaded));
		m_chunks_failed->setText(QString::number(num_failed));
		m_chunks_downloaded->setText(QString::number(num_downloaded));
		m_chunks_not_downloaded->setText(QString::number(num_not_downloaded));
	}
#endif
}

#include "scandlg.moc"

