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

#include <kapplication.h>
#include <kdialog.h>
#include <klocale.h>
#include <ktoolbar.h>
#include <kaction.h>
#include "common.h"

#include <qsizepolicy.h>

#include "transfer.h"

#include "settings.h"
#include "kmainwidget.h"
#include "dlgIndividual.h"


DlgIndividual::DlgIndividual(Transfer * _item):KDialog(0, "dialog")
{

    item = _item;

    //create dock
    m_pDockIndividual =new DockIndividual(this);



    // Actions

    m_paDock = new KToggleAction(i18n("&Dock"),"tool_dock.png", 0, this, SLOT(slotToggleDock()), this, "dockIndividual");


    QVBoxLayout *topLayout = new QVBoxLayout( this, KDialog::marginHint(),KDialog::spacingHint() );
    topLayout->addStrut( 360 );   // makes dlg at least that wide
    topLayout->setResizeMode(QVBoxLayout::Fixed);

    QGridLayout *grid = new QGridLayout( 2, 3 );
    topLayout->addLayout(grid);
    grid->addColSpacing(1, KDialog::spacingHint());

    grid->addWidget(new QLabel(i18n("Source:"), this), 0, 0);

    sourceLabel = new KSqueezedTextLabel(this);
    grid->addWidget(sourceLabel, 0, 2);
    sourceLabel->setText("Source Label");
    grid->addWidget(new QLabel(i18n("Destination:"), this), 1, 0);

    destLabel = new KSqueezedTextLabel(this);
    grid->addWidget(destLabel, 1, 2);
    destLabel->setText("Source Label");

    m_pProgressBar = new KProgress(this);
    topLayout->addWidget( m_pProgressBar );

    // processed info
    QHBoxLayout *hBox = new QHBoxLayout();
    topLayout->addLayout(hBox);

    sizeLabel = new QLabel(this);
    hBox->addWidget(sizeLabel);
    resumeLabel = new QLabel(this);
    hBox->addWidget(resumeLabel);

    hBox = new QHBoxLayout();
    topLayout->addLayout(hBox);

    speedLabel = new QLabel(this);
    hBox->addWidget(speedLabel, 1);
    speedLabel->setText("0 B/s");

    // setup toolbar
    hBox = new QHBoxLayout();
    topLayout->addLayout(hBox);

    KToolBar *toolBar = new KToolBar(this);

    toolBar->setIconText(KToolBar::IconOnly);
    toolBar->setBarPos(KToolBar::Bottom);
    toolBar->enableFloating(false);
    toolBar->enableMoving(false);

    topLayout->addWidget( toolBar );

    // insert toolbar actions
    item->m_paResume->plug(toolBar);
    item->m_paPause->plug(toolBar);
    item->m_paDelete->plug(toolBar);

    toolBar->insertLineSeparator();

    item->m_paQueue->plug(toolBar);
    item->m_paTimer->plug(toolBar);
    item->m_paDelay->plug(toolBar);

    toolBar->insertLineSeparator();
    m_paDock->plug(toolBar);





    QCheckBox * keepOpen = new QCheckBox( i18n("&Keep this window open after the operation is complete."), this);
    connect( keepOpen, SIGNAL( toggled(bool) ), SLOT( slotKeepOpenToggled(bool) ) );
    topLayout->addWidget(keepOpen);

    QFrame *line3 = new QFrame( this );
    line3->setFrameShape( QFrame::HLine );
    line3->setFrameShadow( QFrame::Sunken );
    topLayout->addWidget( line3 );

    hBox = new QHBoxLayout();
    topLayout->addLayout(hBox);

    openFile = new KPushButton( i18n("Open &File"), this );
    connect( openFile, SIGNAL( clicked() ), SLOT( slotOpenFile() ) );
    hBox->addWidget( openFile );
    openFile->setEnabled(false);

    openLocation = new KPushButton( i18n("Open &Destination"), this );
    connect( openLocation, SIGNAL( clicked() ), SLOT( slotOpenLocation() ) );
    hBox->addWidget( openLocation );

    hBox->addStretch(1);

    pbAdvanced = new KPushButton( i18n("Advanced"), this );

    pbAdvanced->setToggleButton(true);

    connect(pbAdvanced, SIGNAL(toggled(bool)), SLOT(slotToggleAdvanced(bool)));

    hBox->addWidget( pbAdvanced );


    // setup tab dialog
    panelAdvanced = new QTabWidget(this);

    // if the time was already set somewhere in the future, keep it
    // otherwise set it to the current time
    QDateTime dt;

    if (item->getStartTime() < QDateTime::currentDateTime() && item->getMode() != Transfer::MD_SCHEDULED)
    {
        dt = QDateTime::currentDateTime();
    }
    else
    {
        dt = item->getStartTime();
    }

    spins = new BWDateTime(dt, this, "spins");

    panelAdvanced->addTab(spins, i18n("Timer"));
    panelAdvanced->hide();

    connect(spins, SIGNAL(signalDateChanged(const QDateTime &)), item, SLOT(slotStartTime(const QDateTime &)));

    // adding item log
    ml_log = new QTextEdit(panelAdvanced);
    ml_log->setReadOnly(true);
    // ml_log->setFixedSize(sizeHint());
    ml_log->setVScrollBarMode(QScrollView::Auto);
    ml_log->setWordWrap(QTextEdit::NoWrap);

    // ml_log->setSizePolicy(policy);

    panelAdvanced->addTab(ml_log, i18n("Log"));
    // panelAdvanced->setFixedSize(sizeHint());



    topLayout->addWidget(panelAdvanced);
    slotToggleAdvanced(ksettings.b_advancedIndividual);
    setFixedSize(sizeHint());
    if (ksettings.b_showIndividual)
    {
        show();
    }




    resize( sizeHint() );
    setMaximumHeight(sizeHint().height());

    //bool keepOpenChecked = false;
    //bool noCaptionYet = true;
    setCaption(i18n("Progress Dialog")); // show something better than kio_uiserver
}


void DlgIndividual::setTotalSize(unsigned long bytes)
{
    m_iTotalSize = bytes;
}


void DlgIndividual::setPercent(unsigned long percent)
{
    m_pProgressBar->setValue(percent);
    m_pDockIndividual->setValue(percent);
    setCaption(i18n("%1% of %2 - %3").arg(percent).arg(KIO::convertSize(m_iTotalSize)).arg(m_location.fileName()));
}


void DlgIndividual::setProcessedSize(unsigned long bytes)
{

    sizeLabel->setText(i18n("%1 of %2").arg(KIO::convertSize(bytes)).arg(KIO::convertSize(m_iTotalSize)));

}


void DlgIndividual::setSpeed(unsigned long bytes_per_second, QTime remaining)
{
    QString msg;
    if (bytes_per_second == 0 && item->getStatus() < Transfer::ST_RUNNING)
        msg=i18n("Stalled");
    else if (bytes_per_second == 0 && item->getStatus() == Transfer::ST_FINISHED)
        msg=i18n("Finished");
    else
        msg=i18n("%1/s ( %2 )").arg(KIO::convertSize(bytes_per_second)).arg(remaining.toString());

    speedLabel->setText(msg);
    m_pDockIndividual->setTip(msg);
}


void DlgIndividual::setCopying(const KURL & from, const KURL & to)
{

    m_location=to;
    setCaption(m_location.fileName());

    sourceLabel->setText(from.url());
    destLabel->setText(to.url());
}


void DlgIndividual::setCanResume(bool resume)
{
    if (resume)
        resumeLabel->setText(i18n("Resumed"));
    else
        resumeLabel->setText(i18n("Not resumed"));
}

void DlgIndividual::slotToggleAdvanced(bool advanced)
{
#ifdef _DEBUG
    sDebugIn<<endl;
#endif

    if (advanced)
        panelAdvanced->show();
    else
        panelAdvanced->hide();

#ifdef _DEBUG
    sDebugOut<<endl;
#endif

}

void DlgIndividual::slotToggleDock()
{
#ifdef _DEBUG
    sDebugIn<<endl;
#endif

    if (m_paDock->isChecked())
    {
        m_pDockIndividual->show();
        hide();
    }
    else
        m_pDockIndividual->hide();

#ifdef _DEBUG
    sDebugOut<<endl;
#endif

}



/** Append the _msg to the log file */
void DlgIndividual::addLog(const QString & _msg)
{

    QString tmps;

    tmps = "<code><font color=\"blue\">" + QTime::currentTime().toString() + "</font> : " + _msg + "</code><br>";

    ml_log->append(tmps);
}


void DlgIndividual::slotKeepOpenToggled(bool bToggled){
#ifdef _DEBUG
    sDebugIn<<endl;
#endif


    bKeepDlgOpen=bToggled;

    if (!bKeepDlgOpen && item->getStatus()==Transfer::ST_FINISHED)
        hide();

#ifdef _DEBUG
    sDebugOut<<endl;
#endif

}


void DlgIndividual::slotOpenLocation(){

#ifdef _DEBUG
    sDebugIn<<endl;
#endif

    KURL location=m_location;
    KProcess proc;
    location.setFileName("");
    proc << "konqueror" << location.prettyURL();
    proc.start(KProcess::DontCare);


#ifdef _DEBUG
    sDebugOut<<endl;
#endif

}

void DlgIndividual::slotOpenFile(){
#ifdef _DEBUG
    sDebugIn<<endl;
#endif

    KProcess proc;
    proc << "konqueror" << m_location.prettyURL();
    proc.start(KProcess::DontCare);


#ifdef _DEBUG
    sDebugOut<<endl;
#endif

}


void DlgIndividual::enableOpenFile(){

#ifdef _DEBUG
    sDebugIn<<endl;
#endif


    openFile->setEnabled(true);

    if (!bKeepDlgOpen)
        hide();

#ifdef _DEBUG
    sDebugOut<<endl;
#endif

}

#include "dlgIndividual.moc"
