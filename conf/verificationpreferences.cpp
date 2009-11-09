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

#include <QtGui/QStringListModel>

#include <KConfigDialog>
#include <KDialog>
#include <KLocale>
#include <KUrlRequester>

VerificationPreferences::VerificationPreferences(KConfigDialog *parent, Qt::WindowFlags f)
  : QWidget(parent, f)
{
    ui.setupUi(this);

    m_tempKeyServers = Settings::signatureKeyServers();
    m_keyServers = new QStringListModel(m_tempKeyServers, this);
    ui.keyServers->setModel(m_keyServers);
    ui.add->setGuiItem(KStandardGuiItem::add());
    ui.remove->setGuiItem(KStandardGuiItem::remove());
    ui.up->setIcon(KIcon("arrow-up"));
    ui.down->setIcon(KIcon("arrow-down"));

#ifndef HAVE_QGPGME
    ui.signatureGroup->hide();
#endif

    slotAutomaticChecksumVerification(Settings::checksumAutomaticVerification());
    slotUrlClicked();

    connect(ui.kcfg_ChecksumAutomaticVerification, SIGNAL(clicked(bool)), this, SLOT(slotAutomaticChecksumVerification(bool)));
    connect(ui.keyServers->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(slotUrlClicked(QItemSelection,QItemSelection)));
    connect(ui.add, SIGNAL(clicked(bool)), this, SLOT(slotAddClicked()));
    connect(ui.remove, SIGNAL(clicked(bool)), this, SLOT(slotRemoveMirror()));
    connect(ui.up, SIGNAL(clicked(bool)), this, SLOT(slotMoveMirrorUp()));
    connect(ui.down, SIGNAL(clicked(bool)), this, SLOT(slotMoveMirrorDown()));

    connect(parent, SIGNAL(accepted()), SLOT(slotAccpeted()));
    connect(parent, SIGNAL(rejected()), SLOT(slotRejected()));
    connect(parent, SIGNAL(defaultClicked()), SLOT(slotDefaultClicked()));
}

void VerificationPreferences::slotAutomaticChecksumVerification(bool enalbed)
{
    ui.kcfg_ChecksumStrength->setEnabled(enalbed);
}

void VerificationPreferences::slotAccpeted()
{
    m_tempKeyServers = m_keyServers->stringList();
    Settings::self()->setSignatureKeyServers(m_tempKeyServers);
    Settings::self()->writeConfig();
}

void VerificationPreferences::slotRejected()
{
    //PreferencesDialog is not recreated, so we have to manually stop the
    //settings from changing
    m_keyServers->setStringList(m_tempKeyServers);
}

void VerificationPreferences::slotUrlClicked(const QItemSelection &selected, const QItemSelection &deslected)
{
    Q_UNUSED(deslected)

    const QModelIndexList indexes = selected.indexes();
    ui.remove->setEnabled(indexes.count() == 1);

    bool up = false;
    bool down = false;
    if (indexes.count() == 1) {
        const QModelIndex index = indexes.first();
        if (index.row() > 0) {
            up = true;
        }
        if (m_keyServers->rowCount() > (index.row() + 1)) {
            down = true;
        }
    }
    ui.up->setEnabled(up);
    ui.down->setEnabled(down);
}

void VerificationPreferences::slotAddClicked()//TODO make better --> own class dialog, to check if it is empty, existing etc.
{
    KDialog *dialog = new KDialog;
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    QWidget *widget = new QWidget(dialog);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    QLabel *label = new QLabel(i18n("Enter an URL to a keyserver."), widget);
    layout->addWidget(label);
    m_url = new KUrlRequester(widget);
    layout->addWidget(m_url);
    dialog->setMainWidget(widget);
    dialog->setCaption(i18n("Key Server"));
    dialog->setButtons(KDialog::Ok);
    connect(dialog, SIGNAL(accepted()), this, SLOT(slotAddMirror()));

    dialog->show();
}

void VerificationPreferences::slotAddMirror()
{
    QStringList data = m_keyServers->stringList();
    data.append(m_url->url().pathOrUrl());
    m_keyServers->setStringList(data);

    emit changed();
}

void VerificationPreferences::slotRemoveMirror()
{
    const QModelIndexList indexes = ui.keyServers->selectionModel()->selectedRows();
    if (indexes.count() == 1) {
        m_keyServers->removeRow(indexes.first().row());
    }

    emit changed();
}

void VerificationPreferences::slotMoveMirrorUp()
{
    moveUrl(true);
}

void VerificationPreferences::slotMoveMirrorDown()
{
    moveUrl(false);
}

void VerificationPreferences::moveUrl(bool moveUp)
{
    const QModelIndexList indexes = ui.keyServers->selectionModel()->selectedRows();
    if (indexes.count() == 1) {
        QModelIndex index = indexes.first();
        const QString url = index.data().toString();
        int row = index.row();
        m_keyServers->removeRow(row);

        if (moveUp) {
            --row;
        } else {
            ++row;
        }
        m_keyServers->insertRow(row);
        index = m_keyServers->index(row);

        m_keyServers->setData(index, url);
        ui.keyServers->setCurrentIndex(index);

        emit changed();
    }
}

void VerificationPreferences::slotDefaultClicked()
{
    KConfigSkeletonItem *item = Settings::self()->findItem("SignatureKeyServers");
    if (item) {
        item->readDefault(Settings::self()->config());
        m_keyServers->setStringList(Settings::signatureKeyServers());
    }
}

#include "verificationpreferences.moc"
