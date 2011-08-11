/***************************************************************************
*   Copyright (C) 2009 Matthias Fuchs <mat69@gmx.net>                     *
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
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
***************************************************************************/

#include "verificationpreferences.h"
#include "settings.h"

#include <KConfigDialog>
#include <KDialog>
#include <KLineEdit>
#include <KLocale>
#include <KUrlRequester>

VerificationPreferences::VerificationPreferences(KConfigDialog *parent, Qt::WindowFlags f)
  : QWidget(parent, f)
{
    ui.setupUi(this);

    m_tempKeyServers = Settings::signatureKeyServers();
    ui.keyservers->upButton()->setText(i18n("&Increase Priority"));
    ui.keyservers->downButton()->setText(i18n("&Decrease Priority"));
    ui.keyservers->setItems(m_tempKeyServers);

#ifndef HAVE_QGPGME
    ui.signatureGroup->hide();
#endif

    connect(ui.keyservers,SIGNAL(changed()),this,SIGNAL(changed()));
    connect(parent, SIGNAL(accepted()), SLOT(slotAccpeted()));
    connect(parent, SIGNAL(rejected()), SLOT(slotRejected()));
    connect(parent, SIGNAL(defaultClicked()), SLOT(slotDefaultClicked()));
}

void VerificationPreferences::slotAccpeted()
{
    ui.keyservers->lineEdit()->clear();
    m_tempKeyServers = ui.keyservers->items();
    Settings::self()->setSignatureKeyServers(m_tempKeyServers);
    Settings::self()->writeConfig();
}

void VerificationPreferences::slotRejected()
{
    //PreferencesDialog is not recreated, so we have to manually stop the
    //settings from changing
    ui.keyservers->setItems(m_tempKeyServers);
    ui.keyservers->lineEdit()->clear();
}

void VerificationPreferences::slotDefaultClicked()
{
    ui.keyservers->lineEdit()->clear();
    KConfigSkeletonItem *item = Settings::self()->findItem("SignatureKeyServers");
    if (item) {
        item->readDefault(Settings::self()->config());
        ui.keyservers->setItems(Settings::signatureKeyServers());
    }
}

#include "verificationpreferences.moc"
