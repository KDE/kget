/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/


#include <klocale.h>
#include <kiconloader.h>

#include "core/transferhandler.h"
#include "mainview.h"
#include "viewscontainer.h"


ViewsContainer::ViewsContainer(QWidget * parent)
    : QWidget(parent)
{
    //Bottom bar layout
    m_HLayout = new QHBoxLayout();
    m_HLayout->setSpacing(1);
    m_HLayout->setMargin(0);

    m_downloads = new QToolButton();
    m_downloads->setToolButtonStyle ( Qt::ToolButtonTextBesideIcon );
    m_downloads->setText(i18n("Downloads"));
    m_downloads->setIcon(SmallIcon("kget"));

    m_finished = new QToolButton();
    m_finished->setToolButtonStyle ( Qt::ToolButtonTextBesideIcon );
    m_finished->setText(i18n("Finished"));
    m_finished->setIcon(SmallIcon("ok"));

    m_HLayout->addWidget(m_downloads);
    m_HLayout->addWidget(m_finished);
    m_HLayout->addSpacing(20);
    m_HLayout->addStretch(1);

    //
    m_VLayout = new QVBoxLayout();
    m_VLayout->setSpacing(1);
    m_VLayout->setMargin(0);
    setLayout(m_VLayout);

    m_SLayout = new QStackedLayout();

    m_VLayout->addLayout(m_SLayout);
    m_VLayout->addLayout(m_HLayout);

    m_mainView = new MainView();
    m_SLayout->addWidget(m_mainView);
}

void ViewsContainer::showTransferDetails(TransferHandler * transfer)
{
    
}

void ViewsContainer::closeTransferDetails(TransferHandler * transfer)
{

}


