/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2007 Urs Wolfer <uwolfer @ kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include "viewscontainer.h"

#include "core/kget.h"
#include "core/plugin/transferfactory.h"
#include "core/transferhandler.h"
#include "core/transfertreeselectionmodel.h"
#include "transfersview.h"
#include "transfersviewdelegate.h"
#include "transferdetails.h"

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
    kDebug(5001) << "ButtonBase::slotToggled" << endl;
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

    QAction * action = m_menu->addAction(KIO::pixmapForUrl( transfer->source(), 0, K3Icon::Desktop, 16 ), filename);
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
            m_transfersMap.erase(prev);
            //Delete the QAction
            delete(prev.key());
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
    setIcon(KIO::pixmapForUrl( transfer->source(), 0, K3Icon::Desktop, 16 ));
    setChecked(true);
}

void TransfersButton::slotToggled(bool checked)
{
    kDebug(5001) << "TransfersButton::slotToggled" << endl;
    if(checked)
    {
        if(m_selectedTransfer)
            emit selectedTransfer(m_selectedTransfer);
    }
}

void TransfersButton::slotActionTriggered(QAction * action)
{
    kDebug(5001) << "Slot action triggered" << endl;

    emit selectedTransfer(m_transfersMap[action]);
}

ViewsContainer::ViewsContainer(QWidget * parent)
    : QWidget(parent)
{
    //Bottom bar layout
    m_HLayout = new QHBoxLayout();
    m_HLayout->setSpacing(2);
    m_HLayout->setMargin(2);

    m_downloadsBt = new ButtonBase();
    m_downloadsBt->setText(i18n("Downloads"));
    m_downloadsBt->setIcon(SmallIcon("kget"));
    m_downloadsBt->setChecked(true);

//     m_finishedBt = new ButtonBase();
//     m_finishedBt->setText(i18n("Old Mainview"));
//     m_finishedBt->setIcon(SmallIcon("ok"));

    m_transfersBt = new TransfersButton();

    m_HLayout->addWidget(m_downloadsBt);
//     m_HLayout->addWidget(m_finishedBt);
    m_HLayout->addSpacing(20);
    m_HLayout->addStretch(1);
    m_HLayout->addWidget(new QLabel(i18n("Transfer details:"), this));
    m_HLayout->addSpacing(10);
    m_HLayout->addWidget(m_transfersBt);

    m_VLayout = new QVBoxLayout();
    m_VLayout->setSpacing(1);
    m_VLayout->setMargin(0);
    setLayout(m_VLayout);

    m_titleBar = new TitleBar();
    m_SLayout = new QStackedLayout();

    QFrame * horizontalLine = new QFrame();
    horizontalLine->setFrameShape(QFrame::HLine);
    horizontalLine->setFrameShadow(QFrame::Sunken);

    m_VLayout->addWidget(m_titleBar);
    m_VLayout->addLayout(m_SLayout);
    m_VLayout->addSpacing(3);
    m_VLayout->addWidget(horizontalLine);
    m_VLayout->addSpacing(3);
    m_VLayout->addLayout(m_HLayout);

    m_transfersView = new TransfersView();
    m_transfersViewDelegate = new TransfersViewDelegate();
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

void ViewsContainer::showTransferDetails(TransferHandler * transfer)
{
    //First check if we already inserted this widget
    QMap<TransferHandler *, QWidget *>::iterator it;
    it = m_transfersMap.find(transfer);

    if( it == m_transfersMap.end() )
    {
        //Create the transfer widget
        QWidget * widget = new TransferDetails(transfer); KGet::factory(transfer)->createDetailsWidget(transfer);
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

void ViewsContainer::closeTransferDetails(TransferHandler * transfer)
{
    m_transfersBt->removeTransfer(transfer);
}

void ViewsContainer::showDownloadsWindow()
{
    kDebug(5001) << "ViewsContainer::showDownloadsWindow" << endl;
    m_SLayout->setCurrentWidget( m_transfersView );

    //TitleBar update
    m_titleBar->setDownloadsWindow();
}

void ViewsContainer::showFinishedWindow()
{
    kDebug(5001) << "ViewsContainer::showFinishedWindow" << endl;
//     m_SLayout->setCurrentWidget( m_finishedView );

    //TitleBar update
    m_titleBar->setFinishedWindow();
}

void ViewsContainer::slotTransferSelected(TransferHandler * transfer)
{
    kDebug(5001) << "SlotTransferSelected" << endl;

    m_SLayout->setCurrentWidget( m_transfersMap[transfer] );

    //TitleBar update
    m_titleBar->setTransfer(transfer);

    //TransfersButton update
    m_transfersBt->setTransfer(transfer);
}

#include "viewscontainer.moc"
