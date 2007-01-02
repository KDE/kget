/* This file is part of the KDE project

   Copyright (C) 2006 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include <QPainter>
#include <QMouseEvent>
#include <QModelIndex>
#include <QToolButton>
#include <QButtonGroup>
#include <QHBoxLayout>

#include <kdebug.h>
#include <klocale.h>
#include <kmenu.h>
#include <kicon.h>

#include "core/kget.h"
#include "core/transferhandler.h"
#include "core/transfergrouphandler.h"
#include "core/transfertreemodel.h"
#include "ui/transfersviewdelegate.h"

GroupStatusEditor::GroupStatusEditor(const TransfersViewDelegate * delegate, QWidget * parent)
    : QWidget(parent),
      m_delegate(delegate)
{
    m_layout = new QHBoxLayout();
    m_layout->addStretch();
    setLayout(m_layout);

    m_btGroup = new QButtonGroup(this);
    m_btGroup->setExclusive(true);

    m_startBt = new QToolButton(this);
    m_startBt->setCheckable(true);
    m_startBt->setAutoRaise(true);
    m_startBt->setIcon(KIcon("player_play"));
    m_startBt->setFixedSize(20, 20);
    m_startBt->installEventFilter(const_cast<TransfersViewDelegate *>(delegate));
    m_layout->addWidget(m_startBt);
    m_btGroup->addButton(m_startBt);

    m_stopBt = new QToolButton(this);
    m_stopBt->setCheckable(true);
    m_stopBt->setAutoRaise(true);
    m_stopBt->setIcon(KIcon("player_pause"));
    m_stopBt->setFixedSize(20, 20);
    m_stopBt->installEventFilter(const_cast<TransfersViewDelegate *>(delegate));
    m_layout->addWidget(m_stopBt);
    m_btGroup->addButton(m_stopBt);

    m_stopBt->setChecked(true);

    m_layout->addStretch();
    m_layout->setMargin(1);

    connect(m_startBt, SIGNAL(toggled(bool)),
            this,      SLOT(slotStatusChanged(bool)));
}

void GroupStatusEditor::setRunning(bool running)
{
    if(running)
        m_startBt->setChecked(true);
    else
        m_stopBt->setChecked(true);
}

bool GroupStatusEditor::isRunning()
{
    return m_startBt->isChecked();
}

void GroupStatusEditor::slotStatusChanged(bool running)
{
    emit const_cast<TransfersViewDelegate *>(m_delegate)->commitData(this);
}

TransfersViewDelegate::TransfersViewDelegate()
    : QItemDelegate(), m_popup(0)
{

}

void TransfersViewDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    const TransferTreeModel * transferTreeModel = static_cast<const TransferTreeModel *>(index.model());

    if(transferTreeModel->isTransferGroup(index))
    {
        painter->save();

        if (option.state & QStyle::State_Selected)
        {
            painter->fillRect(option.rect, option.palette.highlight());
//             painter->setBrush(option.palette.highlightedText());
        }
        else
        {
            painter->fillRect(option.rect, option.palette.alternateBase());
//             painter->setBrush(option.palette.text());
        }

//         painter->setRenderHint(QPainter::Antialiasing, true);
//         painter->setPen(Qt::NoPen);
//         painter->drawText(option.rect, Qt::AlignLeft | Qt::AlignBottom,
//                           transferTreeModel->headerData());

        QItemDelegate::paint(painter, option, index);

        painter->restore();
    }
    else
        QItemDelegate::paint(painter, option, index);
}

void TransfersViewDelegate::drawFocus(QPainter * painter, const QStyleOptionViewItem & option, const QRect & rect)
{
    
}

QSize TransfersViewDelegate::sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    const TransferTreeModel * transferTreeModel = static_cast<const TransferTreeModel *>(index.model());

    if(transferTreeModel->isTransferGroup(index))
    {
        return QSize(0, 35);
    }

    return QSize(0, 24);
}

QWidget * TransfersViewDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem & option , const QModelIndex & index) const
{
    GroupStatusEditor * groupsStatusEditor = new GroupStatusEditor(this, parent);

    return groupsStatusEditor;
}

bool TransfersViewDelegate::editorEvent(QEvent * event, QAbstractItemModel * model, const QStyleOptionViewItem & option, const QModelIndex & index)
{
    QMouseEvent * mouseEvent = dynamic_cast<QMouseEvent *>(event);

    if(mouseEvent)
    {
        if(mouseEvent->button() == Qt::RightButton)
        {
//             kDebug(5001) << "TransfersViewDelegate::editorEvent() -> rightClick" << endl;

            if(m_popup)
            {
                delete(m_popup);
                m_popup = 0;
            }

            TransferTreeModel * transferTreeModel = static_cast<TransferTreeModel *>(model);

            if(transferTreeModel->isTransferGroup(index))
            {
//                 kDebug(5001) << "isTransferGroup = true" << endl;
            }
            else
            {
//                 kDebug(5001) << "isTransferGroup = false" << endl;

                TransferHandler * transferHandler = static_cast<TransferHandler *>(index.internalPointer());

                m_popup = transferHandler->popupMenu(KGet::selectedTransfers());
            }

            if(m_popup)
                m_popup->popup( mouseEvent->globalPos() );
        }
    }

    return false;
}

void TransfersViewDelegate::setEditorData(QWidget * editor, const QModelIndex & index) const
{
    GroupStatusEditor * groupEditor = static_cast<GroupStatusEditor *>(editor);

    groupEditor->setRunning((static_cast<TransferGroupHandler *>(index.internalPointer())->status()) == JobQueue::Running);
}

void TransfersViewDelegate::setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const
{
    GroupStatusEditor * groupEditor = static_cast<GroupStatusEditor *>(editor);

    TransferGroupHandler * groupHandler = static_cast<TransferGroupHandler *>(index.internalPointer());

    if(groupEditor->isRunning())
        groupHandler->start();
    else
        groupHandler->stop();
}


#include "transfersviewdelegate.moc"
