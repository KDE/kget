/* This file is part of the KDE project

   Copyright (C) 2006 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2007 by Javier Goday <jgoday@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "ui/transfersviewdelegate.h"

#include "transferdetails.h"
#include "core/kget.h"
#include "core/transferhandler.h"
#include "core/transfergrouphandler.h"
#include "core/transfertreemodel.h"
#include "core/transfertreeselectionmodel.h"
#include "settings.h"

#include <kdebug.h>
#include <klocale.h>
#include <kmenu.h>
#include <kicon.h>

#include <QApplication>
#include <QPainter>
#include <QMouseEvent>
#include <QModelIndex>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QAbstractItemView>

GroupStatusButton::GroupStatusButton(const QModelIndex & index, QWidget * parent)
    : QToolButton(parent),
      m_status(None),
      m_index(index),
      m_timerId(-1),
      m_iconSize(22),
      m_gradientId(0)
{
}

void GroupStatusButton::checkStateSet()
{
//     kDebug(5001) << "GroupStatusButton::checkStateSet";

    QToolButton::checkStateSet();

    if(isChecked())
    {
        if(m_status == None)
            m_gradientId = 0.7;
        m_status = Selecting;
    }
    else
    {
        if(m_status == None)
            m_gradientId = 1;
        m_status = Deselecting;
    }

    setMouseTracking(!isChecked());

    if(m_timerId == -1)
        m_timerId = startTimer(25);
}

void GroupStatusButton::enterEvent(QEvent * event)
{
    Q_UNUSED(event);
    if(!isChecked())
    {
        m_status = Blinking;

        if(m_timerId == -1)
        {
            m_timerId = startTimer(25);

            if(m_status == !BlinkingExiting)
                m_gradientId = 1;
        }
    }
}

void GroupStatusButton::leaveEvent(QEvent * event)
{
    Q_UNUSED(event);
    if(m_status == Blinking)
        m_status = BlinkingExiting;
}

void GroupStatusButton::paintEvent(QPaintEvent * event)
{
    Q_UNUSED(event);

    QPainter p(this);

    int offset = (event->rect().width() - m_iconSize) / 2;

    if(m_gradientId == 0)
        m_gradientId = isChecked() ? 1 : 0.7;

    QRadialGradient gradient(event->rect().topLeft() + QPoint(event->rect().width() / 2, event->rect().height() / 2), event->rect().width() / 2);

    QPen pen;

    if(KGet::selectionModel()->isSelected(m_index))
    {
        gradient.setColorAt(0, palette().color(QPalette::AlternateBase));
        gradient.setColorAt(m_gradientId, palette().color(QPalette::Highlight));
        gradient.setColorAt(1, palette().color(QPalette::Highlight));
        pen.setColor(palette().color(QPalette::AlternateBase));
    }
    else
    {
        gradient.setColorAt(0, palette().color(QPalette::Highlight));
        gradient.setColorAt(m_gradientId, Qt::transparent);
        gradient.setColorAt(1, Qt::transparent);
        pen.setColor(palette().color(QPalette::Highlight));
    }

    QRect r = event->rect();
    r.adjust(0,0,0,-1);

    p.fillRect(r, gradient);

    p.setRenderHint(QPainter::Antialiasing);

    if(isChecked())
    {
        pen.setWidth(1);
        p.setPen(pen);
        p.drawEllipse(event->rect().x()+5, event->rect().y()+5, event->rect().width()-10, event->rect().width()-10);
    }

    p.drawPixmap(event->rect().topLeft() + QPoint(offset, offset),
                 icon().pixmap(m_iconSize, isChecked() || m_status == Blinking ?
                                           QIcon::Normal : QIcon::Disabled));
}

void GroupStatusButton::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);

    if(m_status == Selecting)
    {
        m_gradientId+=0.05;

        if(m_gradientId >= 1)
        {
            m_status = None;
            m_gradientId = 1;
            killTimer(m_timerId);
            m_timerId = -1;
        }
    }
    else if(m_status == Deselecting)
    {
        m_gradientId-=0.05;

        if(m_gradientId <= 0.7)
        {
            m_status = None;
            m_gradientId = 0.7;
            killTimer(m_timerId);
            m_timerId = -1;
        }
    }
    else if(m_status == Blinking)
    {
        m_gradientId-=0.01;

        if(m_gradientId <= 0.7)
        {
            m_gradientId = 1;
        }
    }
    else if(m_status == BlinkingExiting)
    {
        m_gradientId-=0.01;

        if(m_gradientId <= 0.7)
        {
            m_status = None;
            m_gradientId = 0.7;
            killTimer(m_timerId);
            m_timerId = -1;
        }
    }

    update();
}

GroupStatusEditor::GroupStatusEditor(const QModelIndex & index, const TransfersViewDelegate * delegate, QWidget * parent)
    : QWidget(parent),
      m_delegate(delegate),
      m_index(index)
{
    setMinimumWidth(80);

    m_layout = new QHBoxLayout();
    m_layout->addStretch();
    setLayout(m_layout);

    m_btGroup = new QButtonGroup(this);
    m_btGroup->setExclusive(true);

    m_startBt = new GroupStatusButton(m_index, this);
    m_startBt->setCheckable(true);
    m_startBt->setAutoRaise(true);
    m_startBt->setIcon(KIcon("media-playback-start"));
    m_startBt->setFixedSize(36, 36);
    m_startBt->setIconSize(QSize(22, 22));
    m_startBt->installEventFilter(const_cast<TransfersViewDelegate *>(delegate));
    m_layout->addWidget(m_startBt);
    m_btGroup->addButton(m_startBt);

    m_stopBt = new GroupStatusButton(m_index, this);
    m_stopBt->setCheckable(true);
    m_stopBt->setAutoRaise(true);
    m_stopBt->setIcon(KIcon("media-playback-pause"));
    m_stopBt->setFixedSize(36, 36);
    m_stopBt->setIconSize(QSize(22, 22));
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
    if(running == m_startBt->isChecked())
        return;

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
    Q_UNUSED(running);

    emit const_cast<TransfersViewDelegate *>(m_delegate)->commitData(this);
}

TransfersViewDelegate::TransfersViewDelegate(QAbstractItemView *parent)
    : KExtendableItemDelegate(parent), m_popup(0)
{
    Q_ASSERT(qobject_cast<QAbstractItemView *>(parent));
    setExtendPixmap(SmallIcon("arrow-right"));
    setContractPixmap(SmallIcon("arrow-down"));
    connect(parent, SIGNAL(activated(QModelIndex)), this, SLOT(itemActivated(QModelIndex)));
}

void TransfersViewDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    const TransferTreeModel * transferTreeModel = static_cast<const TransferTreeModel *>(index.model());

    if(transferTreeModel->isTransferGroup(index))
    {
        painter->save();

        if (option.state & QStyle::State_Selected)
        {
        }
        else
        {
            QLinearGradient gradient(option.rect.x(), option.rect.y(),
                                     option.rect.x(), (option.rect.y() + option.rect.height()));
            gradient.setColorAt(0, QApplication::palette().color(QPalette::Base));
            gradient.setColorAt(0.5, QApplication::palette().color(QPalette::AlternateBase).darker(110));
            gradient.setColorAt(1, QApplication::palette().color(QPalette::Base));

            painter->fillRect(option.rect, gradient);
        }

        QItemDelegate::paint(painter, option, index);

        painter->restore();
    }
    else {
        if (KGet::selectionModel()->isSelected(index))
                painter->fillRect(option.rect, QApplication::palette().color(option.state & QStyle::State_Active ?
                                                                            QPalette::Active : QPalette::Inactive,
                                                                            QPalette::Highlight));

        KExtendableItemDelegate::paint(painter, option, index);

        if (index.column() == 3 && !isExtended(transferTreeModel->index(index.row(), 0, index.parent()))) { // the percent column
            TransferHandler *transferHandler = static_cast<TransferHandler *>(index.internalPointer());

            // following progressbar code has mostly been taken from Qt4 examples/network/torrent/mainview.cpp
            // Set up a QStyleOptionProgressBar to precisely mimic the
            // environment of a progress bar.
            QStyleOptionProgressBar progressBarOption;
            progressBarOption.state = QStyle::State_Enabled;
            progressBarOption.direction = QApplication::layoutDirection();
            progressBarOption.rect = option.rect;
            progressBarOption.fontMetrics = QApplication::fontMetrics();
            progressBarOption.minimum = 0;
            progressBarOption.maximum = 100;
            progressBarOption.textAlignment = Qt::AlignCenter;
            progressBarOption.textVisible = true;

            // Set the progress and text values of the style option.
            int progress = transferHandler->percent();
            if (progress >= 0 && progress <= 100) {
                progressBarOption.progress = progress;
                progressBarOption.text = QString().sprintf("%d%%", progressBarOption.progress);
            } else {
                progressBarOption.text = i18nc("not available", "n/a");
            }

            progressBarOption.rect.setY(progressBarOption.rect.y() +
                                        (option.rect.height() - QApplication::fontMetrics().height()) / 2);
            progressBarOption.rect.setHeight(QApplication::fontMetrics().height());

            // Draw the progress bar onto the view.
            QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBarOption, painter);
        }
    }
}

void TransfersViewDelegate::drawFocus(QPainter * painter, const QStyleOptionViewItem & option, const
        QRect & rect) const
{
    Q_UNUSED(painter);
    Q_UNUSED(option);
    Q_UNUSED(rect);
}

QSize TransfersViewDelegate::sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    Q_UNUSED(option);

    const TransferTreeModel * transferTreeModel = static_cast<const TransferTreeModel *>(index.model());

    if(transferTreeModel->isTransferGroup(index))
    {
        return QSize(0, 35);
    }
    else {
        QSize ret(KExtendableItemDelegate::sizeHint(option, index));
        ret.rheight() += 8;
        return ret;
    }
}

QWidget * TransfersViewDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem & option , const QModelIndex & index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    GroupStatusEditor * groupsStatusEditor = new GroupStatusEditor(index, this, parent);

    return groupsStatusEditor;
}

bool TransfersViewDelegate::editorEvent(QEvent * event, QAbstractItemModel * model, const QStyleOptionViewItem & option, const QModelIndex & index)
{
    Q_UNUSED(option);

    QMouseEvent * mouseEvent = dynamic_cast<QMouseEvent *>(event);

    if(mouseEvent)
    {
        if(mouseEvent->button() == Qt::RightButton)
        {
//             kDebug(5001) << "TransfersViewDelegate::editorEvent() -> rightClick";

            if(m_popup)
            {
                delete(m_popup);
                m_popup = 0;
            }

            TransferTreeModel * transferTreeModel = static_cast<TransferTreeModel *>(model);

            if(transferTreeModel->isTransferGroup(index))
            {
//                 kDebug(5001) << "isTransferGroup = true";
                TransferGroupHandler * transferGroupHandler = static_cast<TransferGroupHandler *>(index.internalPointer());

                m_popup = transferGroupHandler->popupMenu();

            }
            else
            {
//                 kDebug(5001) << "isTransferGroup = false";

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
    Q_UNUSED(model);

    GroupStatusEditor * groupEditor = static_cast<GroupStatusEditor *>(editor);

    TransferGroupHandler * groupHandler = static_cast<TransferGroupHandler *>(index.internalPointer());

    if (groupEditor->isRunning())
        groupHandler->start();
    else
        groupHandler->stop();
}

void TransfersViewDelegate::closeExpandableDetails(const QModelIndex &transferIndex)
{
    if(transferIndex.isValid()) {
        contractItem(transferIndex);
        m_editingIndexes.removeAll(transferIndex);
    }
    else {
        foreach(const QModelIndex &index, m_editingIndexes) {
            contractItem(index);
        }

        m_editingIndexes.clear();
    }
}

QWidget *TransfersViewDelegate::getDetailsWidgetForTransfer(TransferHandler *handler)
{
    QGroupBox *groupBox = new QGroupBox(i18n("Transfer details"));

    QVBoxLayout *layout = new QVBoxLayout(groupBox);
    layout->addWidget(TransferDetails::detailsWidget(handler));

    return groupBox;
}

void TransfersViewDelegate::itemActivated(QModelIndex index)
{
    const TransferTreeModel * transferTreeModel = static_cast <const TransferTreeModel *> (index.model());

    if(!transferTreeModel->isTransferGroup(index) && Settings::showExpandableTransferDetails() && index.column() == 0) {
        if(!isExtended(index)) {
            TransferHandler *handler = static_cast <TransferHandler *> (index.internalPointer());
            QWidget *widget = getDetailsWidgetForTransfer(handler);

            m_editingIndexes.append(index);
            extendItem(widget, index);
        }
        else {
            m_editingIndexes.removeAll(index);
            contractItem(index);
        }
    }
}

#include "transfersviewdelegate.moc"
