/***************************************************************************
*                               dlgIndividual.cpp
*                             -------------------
*
*    Revision     : $Id$
*    begin        : Tue Jan 29 2002
*    copyright    : (C) 2002 by Patrick Charbonnier
*                 : Based On Caitoo v.0.7.3 (c) 1998 - 2000, Matej Koss
*    email        : pch@freeshell.org
*
****************************************************************************/

/***************************************************************************
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 ***************************************************************************/


#include <qpushbutton.h>
#include <qtimer.h>
#include <qlayout.h>

#include <kapp.h>
#include <kdialog.h>
#include <klocale.h>
#include <ktoolbar.h>
#include <kaction.h>
#include "common.h"

#include <qsizepolicy.h>

#include "transfer.h"
#include "searchlist.h"
#include "settings.h"
#include "kmainwidget.h"
#include "dlgIndividual.h"


DlgIndividual::DlgIndividual(Transfer * _item):KDialog(0, "dialog")
{
        item = _item;
        QSizePolicy policy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        setSizePolicy(policy);
        QVBoxLayout *topLayout =
                new
                QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());
        topLayout->setResizeMode(QVBoxLayout::Fixed);
        topLayout->addStrut(360);        // makes dlg at least that wide

        QGridLayout *grid = new QGridLayout(3, 3);
        topLayout->addLayout(grid);
        grid->setColStretch(2, 1);
        grid->addColSpacing(1, KDialog::spacingHint());

        grid->addWidget(new QLabel(i18n("Source:"), this), 0, 0);

        sourceLabel = new QLabel(this);
        grid->addWidget(sourceLabel, 0, 2);

        grid->addWidget(new QLabel(i18n("Destination:"), this), 1, 0);

        destLabel = new QLabel(this);
        grid->addWidget(destLabel, 1, 2);

        topLayout->addSpacing(5);

        progressLabel = new QLabel(this);
        grid->addWidget(progressLabel, 2, 2);

#if QT_VERSION < 300
          m_pProgressBar = new KProgress(0, 100, 0, KProgress::Horizontal, this);
#else
         m_pProgressBar = new KProgress(100,this);
#endif

        topLayout->addWidget(m_pProgressBar);

        // processed info
        QHBoxLayout *hBox = new QHBoxLayout();
        topLayout->addLayout(hBox);

        speedLabel = new QLabel(this);
        hBox->addWidget(speedLabel, 1);

        sizeLabel = new QLabel(this);
        hBox->addWidget(sizeLabel);

        resumeLabel = new QLabel(this);
        hBox->addWidget(resumeLabel);

        // setup toolbar
        hBox = new QHBoxLayout();
        topLayout->addLayout(hBox);

        KToolBar *toolBar = new KToolBar(this);

        toolBar->setIconText(KToolBar::IconOnly);
        toolBar->setBarPos(KToolBar::Bottom);
        toolBar->enableFloating(false);
        toolBar->enableMoving(false);

        item->m_paResume->plug(toolBar);
        item->m_paPause->plug(toolBar);
        item->m_paDelete->plug(toolBar);

        toolBar->insertLineSeparator();

        item->m_paQueue->plug(toolBar);
        item->m_paTimer->plug(toolBar);
        item->m_paDelay->plug(toolBar);

        hBox->addWidget(toolBar, 3);

        pbAdvanced = new QPushButton(i18n("Advanced"), this);
        pbAdvanced->setToggleButton(true);
        pbAdvanced->setOn(ksettings.b_advancedIndividual);
        connect(pbAdvanced, SIGNAL(toggled(bool)),
                SLOT(slotToggleAdvanced(bool)));
        hBox->addWidget(pbAdvanced);

        // setup tab dialog
        panelAdvanced = new QTabWidget(this);

        // if the time was already set somewhere in the future, keep it
        // otherwise set it to the current time
        QDateTime dt;
        if (item->getStartTime() < QDateTime::currentDateTime() &&
                        item->getMode() != Transfer::MD_SCHEDULED) {
                dt = QDateTime::currentDateTime();
        } else {
                dt = item->getStartTime();
        }

        spins = new BWDateTime(dt, this, "spins");

        panelAdvanced->addTab(spins, i18n("Timer"));
        panelAdvanced->hide();

        connect(spins, SIGNAL(signalDateChanged(const QDateTime &)), item,
                SLOT(slotStartTime(const QDateTime &)));

        //adding item log
        ml_log = new QTextEdit(panelAdvanced);
        ml_log->setReadOnly(true);
        ml_log->setFixedSize(sizeHint());
        //ml_log->setSizePolicy(policy);

        panelAdvanced->addTab(ml_log, i18n("Log"));
        panelAdvanced->setFixedSize(sizeHint());



        topLayout->addWidget(panelAdvanced);
        slotToggleAdvanced(ksettings.b_advancedIndividual);
        setFixedSize(sizeHint());
        if (ksettings.b_showIndividual) {
                show();
        }


}


void DlgIndividual::setTotalSize(unsigned long bytes)
{
        m_iTotalSize = bytes;
}


void DlgIndividual::setTotalFiles(unsigned long files)
{
        m_iTotalFiles = files;
}


void DlgIndividual::setTotalDirs(unsigned long dirs)
{
        m_iTotalDirs = dirs;
}


void DlgIndividual::setPercent(unsigned long percent)
{
        m_pProgressBar->setValue(percent);

        setCaption(i18n("%1% of %2 - %3").arg(percent).
                   arg(KIO::convertSize(m_iTotalSize)).arg(m_sFilename.
                                                           ascii()));
}


void DlgIndividual::setProcessedSize(unsigned long bytes)
{

        sizeLabel->setText(i18n("%1 of %2").arg(KIO::convertSize(bytes)).
                           arg(KIO::convertSize(m_iTotalSize)));

}


void DlgIndividual::setProcessedDirs(unsigned long dirs)
{
        m_iProcessedDirs = dirs;

        QString tmps;
        tmps =
                i18n("%1 / %2 directories  ").arg(m_iProcessedDirs).
                arg(m_iTotalDirs);
        tmps +=
                i18n("%1 / %2 files").arg(m_iProcessedFiles).arg(m_iTotalFiles);

        progressLabel->setText(tmps);

}


void DlgIndividual::setProcessedFiles(unsigned long files)
{
        m_iProcessedFiles = files;

        QString tmps;
        if (m_iTotalDirs > 1) {
                tmps =
                        i18n("%1 / %2 directories  ").arg(m_iProcessedDirs).
                        arg(m_iTotalDirs);
        }
        tmps +=
                i18n("%1 / %2 files").arg(m_iProcessedFiles).arg(m_iTotalFiles);

        progressLabel->setText(tmps);

}


void DlgIndividual::setSpeed(unsigned long bytes_per_second,
                             QTime remaining)
{
        if (bytes_per_second == 0 && item->getStatus() < Transfer::ST_RUNNING) {

                speedLabel->setText(i18n("Stalled"));

        } else if (bytes_per_second == 0
                        && item->getStatus() == Transfer::ST_FINISHED) {
                speedLabel->setText(i18n("Finished"));
        } else {

                speedLabel->setText(i18n("%1/s ( %2 )").
                                    arg(KIO::convertSize(bytes_per_second)).
                                    arg(remaining.toString()));

        }



}


void DlgIndividual::setCopying(const KURL & from, const KURL & to)
{
        QString source;
        QString dest;

        source = from.url();
        dest = to.url();

        if (source.length() > 50) {
                QString left = source.left(25);
                left = left.left(left.findRev("/") + 1);
                left += "...";
                QString right = source.right(25);
                right = right.right(right.length() - right.find("/"));
                source = left + right;
        }
        if (dest.length() > 50) {
                QString left = dest.left(25);
                left = left.left(left.findRev("/") + 1);
                left += "...";
                QString right = dest.right(25);
                right = right.right(right.length() - right.find("/"));
                dest = left + right;
        }


        m_sFilename = to.fileName();

        setCaption(m_sFilename);

        sourceLabel->setText(source);
        destLabel->setText(dest);

}


void DlgIndividual::setCanResume(bool resume)
{
        if (resume) {
                resumeLabel->setText(i18n("Resumable"));
        } else {
                resumeLabel->setText(i18n("Not resumable"));
        }
}

void DlgIndividual::slotToggleAdvanced(bool advanced)
{
        if (advanced)
                panelAdvanced->show();
        else
                panelAdvanced->hide();

}



/** Append the _msg to the log file */
void DlgIndividual::addLog(const QString & _msg)
{

        QString tmps;

        tmps =
                "<code><font color=\"blue\">" + QTime::currentTime().toString() +
                "</font> : <strong>" + _msg + "</strong></code><br>";

        ml_log->append(tmps);
}

#include "dlgIndividual.moc"


