/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2007 Urs Wolfer <uwolfer @ kde.org>
   Copyright (C) 2010 Matthias Fuchs <mat69@gmx.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "viewscontainer.h"

#include "core/kget.h"
#include "core/transferhandler.h"
#include "core/transfertreeselectionmodel.h"
#include "core/transfertreemodel.h"
#include "transfersview.h"
#include "transfersviewdelegate.h"

#include <QVBoxLayout>

ViewsContainer::ViewsContainer(QWidget * parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout();
    layout->setSpacing(1);
    layout->setMargin(0);

    m_transfersView = new TransfersView(this);
    TransfersViewDelegate *transfersViewDelegate = new TransfersViewDelegate(m_transfersView);
    m_transfersView->setItemDelegate(transfersViewDelegate);
    m_transfersView->setModel(KGet::model());
    m_transfersView->setSelectionModel(KGet::selectionModel());
    m_transfersView->setSelectionMode(QAbstractItemView::ExtendedSelection);

    layout->addWidget(m_transfersView);
    setLayout(layout);
}

void ViewsContainer::showTransferDetails(TransferHandler * transfer)
{
    TransferTreeModel *model = KGet::model();
    m_transfersView->slotItemActivated(model->itemFromHandler(transfer)->index());
}

void ViewsContainer::closeTransferDetails(TransferHandler * transfer)
{
    TransferTreeModel *model = KGet::model();
    m_transfersView->closeExpandableDetails(model->itemFromHandler(transfer)->index());
}

void ViewsContainer::selectAll()
{
    m_transfersView->selectAll();
}

#include "viewscontainer.moc"
