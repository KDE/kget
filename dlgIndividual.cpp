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


#include <qlayout.h>

#include "dockindividual.h"

#include <qlabel.h>
#include <qtabwidget.h>
#include <kprogress.h>
#include <qtextedit.h>
#include <qcheckbox.h>
#include <kpushbutton.h>
#include <ksqueezedtextlabel.h>
#include <kdatetimewidget.h>

#include <kapplication.h>
#include <kaction.h>
#include <klocale.h>
#include <ktoolbar.h>
#include <krun.h>
#include "common.h"

#include "transfer.h"

#include "settings.h"
#include "dlgIndividual.h"


DlgIndividual::DlgIndividual(Transfer * _item)
    : QWidget(0, "dialog", WDestructiveClose)
{
    item = _item;

    //create dock
    m_pDockIndividual =new DockIndividual(this);


    // Actions

    m_paDock = new KToggleAction(i18n("&Dock"),"tool_dock.png", 0, this, SLOT(slotToggleDock()), this, "dockIndividual");


    QVBoxLayout *topLayout = new QVBoxLayout( this, KDialog::marginHint(),KDialog::spacingHint() );
    topLayout->addStrut( 360 );   // makes dlg at least that wide

    QGridLayout *grid = new QGridLayout( 2, 3 );
    topLayout->addLayout(grid);
    grid->addColSpacing(1, KDialog::spacingHint());

    grid->addWidget(new QLabel(i18n("Source:"), this), 0, 0);

    sourceLabel = new KSqueezedTextLabel(this);
    grid->addWidget(sourceLabel, 0, 2);
    sourceLabel->setText(i18n("Source Label"));
    grid->addWidget(new QLabel(i18n("Destination:"), this), 1, 0);

    destLabel = new KSqueezedTextLabel(this);
    grid->addWidget(destLabel, 1, 2);
    destLabel->setText(i18n("Source Label"));

    m_pProgressBar = new KProgress(this);
    topLayout->addWidget( m_pProgressBar );

    // processed info
    QHBoxLayout *hBox = new QHBoxLayout();
    topLayout->addLayout(hBox);

    sizeLabel = new QLabel(this);
    hBox->addWidget(sizeLabel);
    resumeLabel = new QLabel(this);
    hBox->addWidget(resumeLabel);

    speedLabel = new QLabel(this);
    speedLabel->setText(i18n("0 B/s"));
    topLayout->addWidget(speedLabel);

    // setup toolbar
    KToolBar *toolBar = new KToolBar(this);
    toolBar->setIconText(KToolBar::IconOnly);
    toolBar->setBarPos(KToolBar::Bottom);
    toolBar->setMovingEnabled(false);
    toolBar->setFlat(true);

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

    connect(pbAdvanced, SIGNAL(clicked()), SLOT(slotToggleAdvanced()));

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

    spins = new KDateTimeWidget(dt, this, "spins");

    panelAdvanced->addTab(spins, i18n("Timer"));
    panelAdvanced->hide();

    connect(spins, SIGNAL(valueChanged(const QDateTime &)), item, SLOT(slotStartTime(const QDateTime &)));

    // adding item log
    ml_log = new QTextEdit(panelAdvanced);
    ml_log->setTextFormat(LogText);
    ml_log->setReadOnly(true);
    // ml_log->setFixedSize(sizeHint());
    ml_log->setVScrollBarMode(QScrollView::Auto);
    ml_log->setWordWrap(QTextEdit::NoWrap);

    // ml_log->setSizePolicy(policy);

    panelAdvanced->addTab(ml_log, i18n("Log"));
    // panelAdvanced->setFixedSize(sizeHint());



    topLayout->addWidget(panelAdvanced);
    advanced = ksettings.b_advancedIndividual;
    slotToggleAdvanced();

    resize( sizeHint() );

    //bool keepOpenChecked = false;
    //bool noCaptionYet = true;
    setCaption(i18n("Progress Dialog"));

    bKeepDlgOpen=false;
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

    sourceLabel->setText(from.prettyURL());
    destLabel->setText(to.prettyURL());
}


void DlgIndividual::setCanResume(bool resume)
{
    if (resume)
        resumeLabel->setText(i18n("Resumed"));
    else
        resumeLabel->setText(i18n("Not resumed"));
}

//void DlgIndividual::slotToggleAdvanced(bool advanced)
void DlgIndividual::slotToggleAdvanced()
{
#ifdef _DEBUG
    sDebugIn<<endl;
#endif

    if (advanced)
        panelAdvanced->show();
    else
    {
        panelAdvanced->hide();
        adjustSize();
    }
    advanced = !advanced;

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


/** Sets the whole log */
void DlgIndividual::setLog(const QString & _msg)
{
    ml_log->setText(_msg);
}

void DlgIndividual::appendLog(const QString & _msg)
{
    ml_log->append(_msg);
}


void DlgIndividual::slotKeepOpenToggled(bool bToggled)
{
#ifdef _DEBUG
  sDebugIn <<"bToggled= "<<bToggled<<endl;
#endif


    bKeepDlgOpen=bToggled;

    if (!bKeepDlgOpen && item->getStatus()==Transfer::ST_FINISHED)
    {
        hide();
        m_pDockIndividual->hide();
    }

#ifdef _DEBUG
    sDebugOut<<endl;
#endif
}


void DlgIndividual::slotOpenLocation()
{
#ifdef _DEBUG
    sDebugIn<<endl;
#endif

    KURL location=m_location;
    location.setFileName("");
    kapp->invokeBrowser( location.url() );

#ifdef _DEBUG
    sDebugOut<<endl;
#endif
}

void DlgIndividual::slotOpenFile()
{
#ifdef _DEBUG
  sDebugIn "Starting kfmclient with url "<<m_location.prettyURL()<<endl;
#endif

    (void) new KRun( m_location );

#ifdef _DEBUG
    sDebugOut<<endl;
#endif
}


void DlgIndividual::enableOpenFile()
{
#ifdef _DEBUG
    sDebugIn<<endl;
#endif


    openFile->setEnabled(true);

    if (!bKeepDlgOpen)
    {
        hide();
        m_pDockIndividual->hide();
    }

#ifdef _DEBUG
    sDebugOut<<endl;
#endif
}

#include "dlgIndividual.moc"
