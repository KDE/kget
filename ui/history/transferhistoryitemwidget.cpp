/* This file is part of the KDE project

   Copyright (C) 2007 by Lukas Appelhans <l.appelhans@gmx.de>
   Copyright (C) 2008 Javier Goday <jgoday@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "transferhistoryitemwidget.h"
#include "ui/newtransferdialog.h"
#include "ui/history/transferhistory.h"

#include <kio/netaccess.h>
#include <kio/global.h>
#include <KIconLoader>
#include <KLocale>
#include <KIcon>
#include <KRun>

#include <QDate>
#include <QGridLayout>
#include <QLabel>
#include <QMenu>
#include <QModelIndex>
#include <QUrl>

TransferHistoryItemWidget::TransferHistoryItemWidget(QWidget * parent)
    : QFrame(parent),
    m_index(),
    m_url()
{
    setAutoFillBackground(true);

    m_size = new QLabel(this);
    m_date = new QLabel(this);
    m_image = new QLabel(this);
    m_name = new QLabel(this);
    m_host = new QLabel(this);

    m_image->setAlignment(Qt::AlignHCenter);
    m_host->setAlignment(Qt::AlignRight);

    QGridLayout *layout = new QGridLayout();
    layout->addWidget(m_size, 0, 0);
    layout->addWidget(m_date, 0, 1);
    layout->addWidget(m_image, 1, 0, 1, 2);
    layout->addWidget(m_name, 2, 0, 1, 2);
    layout->addWidget(m_host, 3, 0, 1, 2);

    setLayout(layout);

    // css styles
    setObjectName("box");
    setStyleSheet("#box {border: 1px solid #bababa;background-color:#f7f7f7;}"
                "#box:hover{border:1px solid black;background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
                " stop:0 white, stop: 0.4 lightgray, stop:1 white);}");
    m_size->setStyleSheet("font-size:x-small; color: #707070");
    m_date->setStyleSheet("font-size:x-small; color: #707070");
    m_host->setStyleSheet("font-size: 8px;color: #424d0c");
    m_name->setStyleSheet("font-size: 9px;");

    // Actions
    //m_actionDownload = actionCollection()->addAction("actionDownload");
    m_actionDownload = new QAction(this);
    m_actionDownload->setText(i18n("Download again"));
    m_actionDownload->setIcon(KIcon("document-new"));
    connect(m_actionDownload, SIGNAL(triggered()), SLOT(slotDownload()));

    // m_actionDelete_Selected = actionCollection()->addAction("actionDelete_Selected");
    m_actionDelete_Selected = new QAction(this);
    m_actionDelete_Selected->setText(i18n("Delete selected"));
    m_actionDelete_Selected->setIcon(KIcon("edit-delete"));
    connect(m_actionDelete_Selected, SIGNAL(triggered()), SLOT(slotDeleteTransfer()));

    // m_openFile = actionCollection()->addAction("openFile");
    m_openFile = new QAction(this);
    m_openFile->setText(i18n("Open file"));
    m_openFile->setIcon(KIcon("document-open"));
    connect(m_openFile, SIGNAL(triggered()), SLOT(slotOpenFile()));
}

TransferHistoryItemWidget::~TransferHistoryItemWidget()
{
}

void TransferHistoryItemWidget::setSize(int size)
{
    m_size->setText(KIO::convertSize(size));
}

void TransferHistoryItemWidget::setDate(const QDate &date)
{
    m_date->setText(date.toString("dd.MM.yyyy"));
}

void TransferHistoryItemWidget::setUrl(const QString &transfer)
{
    m_url = transfer;
    QUrl url(transfer);

    m_host->setText(url.host());
    // FIXME: find another way of get the file name for a path
    m_name->setText(url.path().mid(url.path().lastIndexOf("/") + 1));
    setToolTip(url.path().mid(url.path().lastIndexOf("/") + 1));
}

void TransferHistoryItemWidget::setDest(const QString &dest)
{
    m_dest = dest;

    m_image->setPixmap(KIO::pixmapForUrl(dest, 0, KIconLoader::Panel));
}

void TransferHistoryItemWidget::setModelIndex(const QModelIndex &index)
{
    m_index = index;
}

void TransferHistoryItemWidget::contextMenuEvent(QContextMenuEvent *event)
{
    Q_UNUSED(event)

    QMenu *contextMenu = new QMenu(this);
    contextMenu->addAction(m_actionDownload);
    contextMenu->addAction(m_actionDelete_Selected);
    contextMenu->addAction(m_openFile);

    contextMenu->exec(QCursor::pos());
}

void TransferHistoryItemWidget::slotOpenFile()
{
    new KRun(m_dest, this, true, false);
}

void TransferHistoryItemWidget::slotDownload()
{
    NewTransferDialog::showNewTransferDialog(m_url);
}

void TransferHistoryItemWidget::slotDeleteTransfer()
{
    emit deletedTransfer(m_url, m_index);
}

#include "transferhistoryitemwidget.moc"
