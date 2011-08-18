/* This file is part of the KDE project

   Copyright (C) 2008 Urs Wolfer <uwolfer @ kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "dlgwebinterface.h"

#include "settings.h"

#include <KMessageBox>
#include <kwallet.h>

DlgWebinterface::DlgWebinterface(KDialog *parent)
    : QWidget(parent),
      m_wallet(0)
{
    setupUi(this);

    readConfig();
    
    connect(parent, SIGNAL(accepted()), SLOT(saveSettings()));
    connect(webinterfacePwd, SIGNAL(textChanged(QString)), SIGNAL(changed()));
}

DlgWebinterface::~DlgWebinterface()
{
    delete m_wallet;
}

void DlgWebinterface::readConfig()
{
    if (Settings::webinterfaceEnabled()) {
        m_wallet = KWallet::Wallet::openWallet(KWallet::Wallet::LocalWallet(),
                                               winId(),///Use MainWindow?
                                               KWallet::Wallet::Asynchronous);
        if (m_wallet) {
            connect(m_wallet, SIGNAL(walletOpened(bool)), SLOT(walletOpened(bool)));
        } else {
            KMessageBox::error(0, i18n("Could not open KWallet"));
        }
    }
}

void DlgWebinterface::walletOpened(bool opened)
{
    if (opened &&
        (m_wallet->hasFolder("KGet") ||
         m_wallet->createFolder("KGet")) &&
         m_wallet->setFolder("KGet")) {
        QString pwd;
        m_wallet->readPassword("Webinterface", pwd);
        webinterfacePwd->setText(pwd);
    } else {
        KMessageBox::error(0, i18n("Could not open KWallet"));
    }
}

void DlgWebinterface::saveSettings()
{
    if (m_wallet) {
        m_wallet->writePassword("Webinterface", webinterfacePwd->text());
    }
    emit saved();
}

#include "dlgwebinterface.moc"
