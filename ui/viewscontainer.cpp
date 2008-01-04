/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2007 Urs Wolfer <uwolfer @ kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "viewscontainer.h"

#include "core/kget.h"
#include "core/plugin/transferfactory.h"
#include "core/transferhandler.h"
#include "core/transfertreeselectionmodel.h"
#include "transfersview.h"
#include "transfersviewdelegate.h"
#include "transferdetails.h"
#include "settings.h"

#include <klocale.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kio/global.h>
#include <KTitleWidget>

#include <QMenu>
#include <QFrame>
#include <QLabel>

TitleBar::TitleBar(QWidget * parent)
    : QWidget(parent)
{
    m_titleWidget = new KTitleWidget(this);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(m_titleWidget);
}

void TitleBar::setTransfer(TransferHandler * transfer)
{
    m_titleWidget->setText(i18n("Details for: %1", transfer->source().fileName()), Qt::AlignCenter);
}

void TitleBar::setDownloadsWindow()
{
    m_titleWidget->setText(i18n("All downloads"), Qt::AlignCenter);
}

void TitleBar::setFinishedWindow()
{
    m_titleWidget->setText(i18n("Finished downloads"), Qt::AlignCenter);
}

ButtonBase::ButtonBase(QWidget * parent)
    : QToolButton(parent)
{
    setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    setAutoExclusive(true);
    setAutoRaise(false);
    setCheckable(true);
    setMinimumWidth(100);
    connect(this, SIGNAL(toggled(bool)), SLOT(slotToggled(bool)));
}

void ButtonBase::slotToggled(bool checked)
{
    kDebug(5001);
    if(checked)
    {
        emit activated();
    }
}

TransfersButton::TransfersButton()
    : m_selectedTransfer(0)
{
    setPopupMode(QToolButton::MenuButtonPopup);

    m_menu = new QMenu(this);
    m_menu->setTitle(i18n("Transfers:"));
    setMenu(m_menu);

    setEnabled(false);

    connect(m_menu, SIGNAL(triggered(QAction *)), 
            this,   SLOT(slotActionTriggered(QAction *)));
}

void TransfersButton::addTransfer(TransferHandler * transfer)
{
    QString filename = transfer->source().fileName();

    QAction * action = m_menu->addAction(KIO::pixmapForUrl( transfer->source(), 0, KIconLoader::Desktop, 16 ), filename);
    m_transfersMap[action] = transfer;

    if(!m_selectedTransfer)
    {
        m_selectedTransfer = transfer;
        setText(filename);
        setEnabled(true);
    }
}

void TransfersButton::removeTransfer(TransferHandler * transfer)
{
    QMap<QAction *, TransferHandler *>::iterator it = m_transfersMap.begin();
    while (it != m_transfersMap.end())
    {
        QMap<QAction *, TransferHandler *>::iterator prev = it;
        ++it;
        if (prev.value() == transfer)
        {
            delete(prev.key());
            m_transfersMap.erase(prev);
            //Delete the QAction
        }
    }

    if(m_transfersMap.count() == 0)
    {
        setEnabled(false);
        setText(QString());
    }
    else
    {
        if(m_selectedTransfer == transfer)
        {
            m_selectedTransfer = m_transfersMap.begin().value();
            setText(m_selectedTransfer->source().fileName());
            if(isChecked())
                emit selectedTransfer(m_selectedTransfer);
        }
    }
}

void TransfersButton::setTransfer(TransferHandler * transfer)
{
    m_selectedTransfer = transfer;
    setText(transfer->source().fileName());
    setIcon(KIO::pixmapForUrl( transfer->source(), 0, KIconLoader::Desktop, 16 ));
    setChecked(true);
}

void TransfersButton::slotToggled(bool checked)
{
    kDebug(5001);
    if(checked)
    {
        if(m_selectedTransfer)
            emit selectedTransfer(m_selectedTransfer);
    }
}

void TransfersButton::slotActionTriggered(QAction * action)
{
    kDebug(5001) << "Slot action triggered";

    emit selectedTransfer(m_transfersMap[action]);
}

ViewsContainer::ViewsContainer(QWidget * parent)
    : QWidget(parent)
{
    //Bottom bar layout
    m_bottomBar = new QWidget(this);
    QVBoxLayout *bbVBox = new QVBoxLayout(m_bottomBar);
    bbVBox->setMargin(2);
    bbVBox->setSpacing(2);

    QFrame * horizontalLine = new QFrame();
    horizontalLine->setFrameShape(QFrame::HLine);
    horizontalLine->setFrameShadow(QFrame::Sunken);

    m_HLayout = new QHBoxLayout();

    bbVBox->addWidget(horizontalLine);
    bbVBox->addSpacing(1);
    bbVBox->addLayout(m_HLayout);

    m_bottomBar->setVisible(!Settings::showExpandableTransferDetails());

    m_downloadsBt = new ButtonBase();
    m_downloadsBt->setText(i18n("Downloads"));
    m_downloadsBt->setIcon(SmallIcon("kget"));
    m_downloadsBt->setChecked(true);

//     m_finishedBt = new ButtonBase();
//     m_finishedBt->setText(i18n("Old Mainview"));
//     m_finishedBt->setIcon(SmallIcon("dialog-ok"));

    m_transfersBt = new TransfersButton();

    m_HLayout->addWidget(m_downloadsBt);
//     m_HLayout->addWidget(m_finishedBt);
    m_HLayout->addStretch(1);
    m_HLayout->addWidget(new QLabel(i18n("Transfer details:"), this));
    m_HLayout->addWidget(m_transfersBt);

    m_VLayout = new QVBoxLayout();
    m_VLayout->setSpacing(1);
    m_VLayout->setMargin(0);
    setLayout(m_VLayout);

    m_titleBar = new TitleBar();
    m_titleBar->setVisible(!Settings::showExpandableTransferDetails());
    m_SLayout = new QStackedLayout();

    m_VLayout->addWidget(m_titleBar);
    m_VLayout->addLayout(m_SLayout);
    m_VLayout->addWidget(m_bottomBar);

    m_transfersView = new TransfersView();
    m_transfersViewDelegate = new TransfersViewDelegate(m_transfersView);
    m_transfersView->setItemDelegate(m_transfersViewDelegate);
    KGet::addTransferView(m_transfersView);
    m_transfersView->setSelectionModel(KGet::selectionModel());
    m_transfersView->setSelectionMode(QAbstractItemView::ExtendedSelection);

    m_SLayout->addWidget(m_transfersView);

    //This view hasn't been coded yet. For the moment,
    //I set it to the old MainView.
//     m_finishedView = new MainView();
//     m_finishedView = new QWidget();
//     m_SLayout->addWidget(m_finishedView);

    connect(m_downloadsBt, SIGNAL(activated()),
            this,          SLOT(showDownloadsWindow()));
//     connect(m_finishedBt,  SIGNAL(activated()),
//             this,          SLOT(showFinishedWindow()));
    connect(m_transfersBt, SIGNAL(selectedTransfer(TransferHandler *)),
            this,          SLOT(slotTransferSelected(TransferHandler *)));

    showDownloadsWindow();
}

void ViewsContainer::setExpandableDetails(bool show)
{
    m_bottomBar->setVisible(!show);
    m_titleBar->setVisible(!show);
    if(show) {
        showDownloadsWindow();
    }
    else {
        m_transfersViewDelegate->closeExpandableDetails();
    }
}

void ViewsContainer::showTransferDetails(TransferHandler * transfer)
{
    if(Settings::showExpandableTransferDetails()) {
        m_transfersViewDelegate->itemActivated(m_transfersView->indexFromTransferHandler(transfer));
    }
    else {
        //First check if we already inserted this widget
        QMap<TransferHandler *, QWidget *>::iterator it;
        it = m_transfersMap.find(transfer);

        if( it == m_transfersMap.end() )
        {
            //Create the transfer widget
            QWidget * widget = TransferDetails::detailsWidget(transfer);

            //Add it to the m_transferItems list
            m_transfersMap[transfer] = widget;
            //Add the widget to the qstackedlayout
            m_SLayout->addWidget( widget );
            //Insert a new transfer in the m_transfersBt button
            m_transfersBt->addTransfer(transfer);
        }

        //Select the new transfer
        slotTransferSelected(transfer);
    }
}

void ViewsContainer::closeTransferDetails(TransferHandler * transfer)
{
    if(Settings::showExpandableTransferDetails()) {
        m_transfersViewDelegate->closeExpandableDetails(m_transfersView->indexFromTransferHandler(transfer));
    }
    else {
        m_transfersBt->removeTransfer(transfer);
    }
    m_transfersMap.remove(transfer);
    showDownloadsWindow();
}

void ViewsContainer::showDownloadsWindow()
{
    kDebug(5001) << "ViewsContainer::showDownloadsWindow";
    m_SLayout->setCurrentWidget( m_transfersView );

    //TitleBar update
    m_titleBar->setDownloadsWindow();
}

void ViewsContainer::showFinishedWindow()
{
    kDebug(5001) << "ViewsContainer::showFinishedWindow";
//     m_SLayout->setCurrentWidget( m_finishedView );

    //TitleBar update
    m_titleBar->setFinishedWindow();
}

void ViewsContainer::slotTransferSelected(TransferHandler * transfer)
{
    kDebug(5001) << "SlotTransferSelected";

    m_SLayout->setCurrentWidget( m_transfersMap[transfer] );

    //TitleBar update
    m_titleBar->setTransfer(transfer);

    //TransfersButton update
    m_transfersBt->setTransfer(transfer);
}

#include "viewscontainer.moc"
