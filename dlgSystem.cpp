/***************************************************************************
*                               dlgSystem.cpp
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

#include <qcheckbox.h>
#include <qlabel.h>
#include <qgroupbox.h>
#include <qpushbutton.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>

#ifdef index
#undef index
#endif

#include <kcombobox.h>

#include <kfontdialog.h>
#include <kaudioplayer.h>
//#include <klineeditdlg.h>
#include <kurlrequesterdlg.h>
#include <klocale.h>
#include <kdialog.h>
#include <kio/netaccess.h>
#include <kfiledialog.h>
#include "settings.h"
#include "kmainwidget.h"
#include "dlgSystem.h"


DlgSystem::DlgSystem(QWidget * parent):QWidget(parent, "", 0)
{
    QGridLayout *topGridLayout = new QGridLayout(this, 4, 4, 20, KDialog::spacingHint());

    topGridLayout->setRowStretch(0, 7);
    topGridLayout->setRowStretch(1, 7);
    topGridLayout->setRowStretch(2, 5);
    topGridLayout->setRowStretch(3, 5);

    topGridLayout->setColStretch(0, 5);
    topGridLayout->setColStretch(1, 5);
    topGridLayout->setColStretch(2, 5);
    topGridLayout->setColStretch(3, 5);

    // sound settings

    cb_useSound = new QCheckBox(i18n("Use sounds"), this);
    topGridLayout->addWidget(cb_useSound, 0, 0);

    cmb_sounds = new KComboBox(false, this);

    cmb_sounds->insertItem(i18n("Added"));
    cmb_sounds->insertItem(i18n("Started"));
    cmb_sounds->insertItem(i18n("Finished"));
    cmb_sounds->insertItem(i18n("Finished All"));
    topGridLayout->addWidget(cmb_sounds, 0, 1);

    pb_changesound = new QPushButton(i18n("Change..."), this);
    topGridLayout->addWidget(pb_changesound, 0, 2);
    connect(pb_changesound, SIGNAL(clicked()), SLOT(setupSound()));

    pb_testsound = new QPushButton(i18n("Test"), this);
    topGridLayout->addWidget(pb_testsound, 0, 3);
    connect(pb_testsound, SIGNAL(clicked()), SLOT(testSound()));

    connect(cb_useSound, SIGNAL(toggled(bool)), cmb_sounds, SLOT(setEnabled(bool)));
    connect(cb_useSound, SIGNAL(toggled(bool)), pb_changesound, SLOT(setEnabled(bool)));
    connect(cb_useSound, SIGNAL(toggled(bool)), pb_testsound, SLOT(setEnabled(bool)));

    // animation settings
    cb_useAnimation = new QCheckBox(i18n("Use animation"), this);
    topGridLayout->addMultiCellWidget(cb_useAnimation, 1, 1, 0, 1);

    // window style
    bg_window = new QButtonGroup(i18n("Window Style"), this, "bg_window");
    bg_window->setEnabled( false ); // ### not implemented yet :-/
    topGridLayout->addMultiCellWidget(bg_window, 2, 2, 0, 3);

    QHBoxLayout *hLayout = new QHBoxLayout(bg_window, 20, KDialog::spacingHint());

    rb_normal = new QRadioButton(i18n("Normal"), bg_window);
    bg_window->insert(rb_normal);
    hLayout->addWidget(rb_normal);

    rb_docked = new QRadioButton(i18n("Dock widget"), bg_window);
    bg_window->insert(rb_docked);
    hLayout->addWidget(rb_docked);

    rb_droptarget = new QRadioButton(i18n("Drop target"), bg_window);
    bg_window->insert(rb_droptarget);
    hLayout->addWidget(rb_droptarget);

    // font groupbox
    gb_font = new QGroupBox(this, "gb_font");
    gb_font->setTitle(i18n("Font Settings"));
    topGridLayout->addMultiCellWidget(gb_font, 3, 3, 0, 3);

    QGridLayout *gLayout = new QGridLayout(gb_font, 1, 2, 20, KDialog::spacingHint());

    gLayout->setRowStretch(0, 1);

    gLayout->setColStretch(0, 10);
    gLayout->setColStretch(1, 5);

    lb_font = new QLabel(i18n("Dolor lpse"), gb_font);
    lb_font->setAlignment(AlignHCenter | AlignVCenter);
    lb_font->setBackgroundColor(QColor(white));
    lb_font->setFrameStyle(QFrame::Box | QFrame::Sunken);
    gLayout->addWidget(lb_font, 0, 0);

    pb_browse = new QPushButton(i18n("Change..."), gb_font);
    connect(pb_browse, SIGNAL(clicked()), SLOT(changeFont()));
    gLayout->addWidget(pb_browse, 0, 1);
}


void DlgSystem::setupSound()
{
    sDebugIn << endl;
    QString s, t;

    int id = cmb_sounds->currentItem();

    switch (id)
    {
    case 0:
        s = i18n("Sound filename for action 'added':");
        t = soundAdded;
        break;
    case 1:
        s = i18n("Sound filename for action 'started':");
        t = soundStarted;
        break;
    case 2:
        s = i18n("Sound filename for action 'finished':");
        t = soundFinished;
        break;
    case 3:
        s = i18n("Sound filename for action 'finished-all':");
        t = soundFinishedAll;
        break;
    }
    KURLRequesterDlg *box = new KURLRequesterDlg(t, s, this, "kurl_sound");
    KFileDialog *pDlg = box->fileDialog();
    pDlg->setFilter(i18n("*.wav|WAV files\n*|All files"));
    int result = box->exec();
    sDebug << "Result= " << result << endl;

    if (result == QDialog::Rejected)
    {  /* cancelled */
        sDebug << "Cancelled" << endl;
        delete box;
        return;
    }

    s = box->selectedURL().url();       // text();
    delete box;

    sDebug << "Selected audio files: " << s << endl;
    if (s.isEmpty())
    {          /* answer is "" */
        sDebugOut << "empty selection" << endl;
        return;
    }

    switch (id)
    {
    case 0:
        soundAdded = s;
        break;

    case 1:
        soundStarted = s;
        break;

    case 2:
        soundFinished = s;
        break;

    case 3:
        soundFinishedAll = s;
        break;
    }
    sDebugOut << endl;
}


void DlgSystem::testSound()
{
    sDebug << ">>>>Entering" << endl;

    QString soundFile;

    switch (cmb_sounds->currentItem())
    {
    case 0:
        soundFile = soundAdded;
        break;
    case 1:
        soundFile = soundStarted;
        break;
    case 2:
        soundFile = soundFinished;
        break;
    case 3:
        soundFile = soundFinishedAll;
        break;
    }
    QString tmpFile;
    sDebug << "File to play " << soundFile << endl;
    //   KAudioPlayer::play(soundFile);
    //   KAudioPlayer::play( "/home/pch/pop.wav");

    if (KIO::NetAccess::download(soundFile, tmpFile))
    {
        sDebug << "Temp file to play is " << tmpFile << endl;
        KAudioPlayer::play(tmpFile);
        KIO::NetAccess::removeTempFile(tmpFile);
    }


    sDebug << "<<<<Leaving" << endl;
}


void DlgSystem::changeFont()
{
    QFont font = lb_font->font();

    if (KFontDialog::getFont(font, false, this) == QDialog::Rejected)
        return;
    lb_font->setFont(font);
}


void DlgSystem::setData()
{
    cb_useSound->setChecked(ksettings.b_useSound);
    cmb_sounds->setEnabled(ksettings.b_useSound);
    pb_changesound->setEnabled(ksettings.b_useSound);
    pb_testsound->setEnabled(ksettings.b_useSound);

    soundAdded = ksettings.audioAdded;
    soundStarted = ksettings.audioStarted;
    soundFinished = ksettings.audioFinished;
    soundFinishedAll = ksettings.audioFinishedAll;

    cb_useAnimation->setChecked(ksettings.b_useAnimation);


    lb_font->setFont(ksettings.listViewFont);
}


void DlgSystem::applyData()
{
    if (cb_useSound->isChecked() != ksettings.b_useSound)
    {
        kmain->slotToggleSound();
    }

    QString tmpFile;

    if (KIO::NetAccess::download(soundAdded, tmpFile))
        ksettings.audioAdded = tmpFile;
    if (KIO::NetAccess::download(soundStarted, tmpFile))
        ksettings.audioStarted = tmpFile;
    if (KIO::NetAccess::download(soundFinished, tmpFile))
        ksettings.audioFinished = tmpFile;
    if (KIO::NetAccess::download(soundFinishedAll, tmpFile))
        ksettings.audioFinishedAll = tmpFile;

    if (cb_useAnimation->isChecked() != ksettings.b_useAnimation)
    {
        kmain->slotToggleAnimation();
    }

    ksettings.listViewFont = lb_font->font();
    kmain->setListFont();
}

#include "dlgSystem.moc"
