/* This file is part of the KDE project

   Based in the kcategorizeditemsviewdelegate from kdebase/workspace/libs/plasma/appletbrowser by Ivan Cukic
   Copyright (C) 2008 Javier Goday <jgoday@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "transferhistoryitemdelegate.h"
#include "ui/history/transferhistorycategorizeddelegate.h"
#include "ui/history/transferhistorycategorizedview.h"
#include "ui/newtransferdialog.h"

#include <QAbstractItemModel>
#include <QAction>
#include <QApplication>
#include <QDate>
#include <QMenu>
#include <QModelIndex>
#include <QMouseEvent>
#include <QPainter>

#include <KDebug>
#include <kio/netaccess.h>
#include <kio/global.h>
#include <KIconLoader>
#include <KIcon>
#include <KLocale>
#include <KRun>

TransferHistoryItemDelegate::TransferHistoryItemDelegate(QWidget *parent) : QStyledItemDelegate(),
    m_selectedIndex()
{
    m_view = parent;

    // Actions
    m_actionDownload = new QAction(this);
    m_actionDownload->setText(i18n("Download again"));
    m_actionDownload->setIcon(KIcon("document-new"));
    connect(m_actionDownload, SIGNAL(triggered()), SLOT(slotDownload()));

    m_actionDelete_Selected = new QAction(this);
    m_actionDelete_Selected->setText(i18nc("Delete selected history-item", "Delete selected"));
    m_actionDelete_Selected->setIcon(KIcon("edit-delete"));
    connect(m_actionDelete_Selected, SIGNAL(triggered()), SLOT(slotDeleteTransfer()));

    m_openFile = new QAction(this);
    m_openFile->setText(i18n("Open file"));
    m_openFile->setIcon(KIcon("document-open"));
    connect(m_openFile, SIGNAL(triggered()), SLOT(slotOpenFile()));
}

TransferHistoryItemDelegate::~TransferHistoryItemDelegate()
{
}

void TransferHistoryItemDelegate::paint(QPainter *painter,
        const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!option.state.testFlag(QStyle::State_Selected) && !option.state.testFlag(QStyle::State_MouseOver)) {
        // draw a separator
        painter->save();

        QRect roundRect(option.rect.left() + 1, option.rect.top() + 1,
                        option.rect.width() - 2, option.rect.height() - 2);

        QPainterPath path;
        path.addRoundRect(roundRect, 2, 2);
        QLinearGradient gradient(roundRect.left(), roundRect.top(),
                                 roundRect.left(), roundRect.bottom());
        gradient.setColorAt(0, Qt::transparent);
        gradient.setColorAt(0.95, option.palette.color(QPalette::AlternateBase).darker(130));
        QBrush brush(gradient);

        painter->fillPath(path, brush);

        painter->setPen(option.palette.color(QPalette::AlternateBase).darker(190));
        painter->drawRoundRect(roundRect, 2, 2);
        painter->restore();
    }

    QStyleOptionViewItemV4 opt(option);
    QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);

    const QAbstractItemModel *model = static_cast <const QAbstractItemModel *> (index.model());
    QUrl url(model->data(index, TransferHistoryCategorizedDelegate::RoleUrl).toString());
    QString name = url.path().mid(url.path().lastIndexOf("/") + 1);
    KIcon icon(KIO::pixmapForUrl(
                    model->data(index, TransferHistoryCategorizedDelegate::RoleDest).toString(),
                    0, KIconLoader::Panel));
    QString size = KIO::convertSize(model->data(index, TransferHistoryCategorizedDelegate::RoleSize).toInt());
    QString date = model->data(index, TransferHistoryCategorizedDelegate::RoleDate).toDate().toString("dd.MM.yyyy");
    QString host = url.host();

    // draw the host
    painter->save();
    painter->setPen(option.palette.color(QPalette::Link));
    // draw the host
    painter->drawText(option.rect.left() + PADDING,
                      option.rect.top() + PADDING,
                      option.rect.width() - PADDING * 2, 15,
                      Qt::AlignTop | Qt::AlignLeft, host);
    painter->restore();

    // draw the transfer icon
    icon.paint(painter,
            option.rect.left() + option.rect.width() / 2 - ICON_SIZE / 2,
            option.rect.top() + option.rect.height() / 2 - ICON_SIZE / 2 - PADDING * 2,
            ICON_SIZE, ICON_SIZE, Qt::AlignCenter, QIcon::Active);

    painter->save();
    QColor subcolor = (option.state.testFlag(QStyle::State_Selected) || (option.state.testFlag(QStyle::State_MouseOver))) ?
                    option.palette.color(QPalette::Text) :
                    option.palette.color(QPalette::BrightText);

    // draw a separator line between the file name and his size and date
    painter->setPen(option.palette.color(QPalette::AlternateBase).darker(190));
    painter->drawLine(option.rect.left() + 2, option.rect.bottom() + PADDING - 27,
                      option.rect.right() - 2, option.rect.bottom() + PADDING - 27);
    painter->setPen(subcolor);
     // draw the size
    painter->drawText(option.rect.right() - PADDING - 100,
                       option.rect.bottom() + PADDING - 25,
                       100 - PADDING * 2, 25,
                       Qt::AlignTop | Qt::AlignRight, size);

     // draw the date
    painter->drawText(option.rect.left() + PADDING,
                       option.rect.bottom() + PADDING - 25,
                       100 - PADDING * 2, 25,
                       Qt::AlignTop | Qt::AlignLeft, date);

    painter->restore();

    // draw the filenamne
    painter->save();
    QColor foregroundColor = (option.state.testFlag(QStyle::State_Selected)) ?
        option.palette.color(QPalette::HighlightedText):option.palette.color(QPalette::Text);
    painter->setPen(foregroundColor);
    painter->setFont(option.font);
    painter->drawText(option.rect.left() + PADDING,
                      option.rect.bottom() - PADDING - 35,
                      200 - PADDING * 2, 15,
                      Qt::AlignBottom | Qt::AlignCenter, name);
    painter->restore();
}

QSize TransferHistoryItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)

    return QSize(200, 110);
}

bool TransferHistoryItemDelegate::editorEvent(QEvent * event, QAbstractItemModel * model,
                    const QStyleOptionViewItem &option, const QModelIndex & index)
{
    Q_UNUSED(option)
    Q_UNUSED(model)

    QMouseEvent * mouseEvent = dynamic_cast<QMouseEvent *>(event);

    if(mouseEvent && index.isValid()) {
        if(mouseEvent->button() == Qt::RightButton) {
            m_selectedIndex = index;

            QMenu *contextMenu = new QMenu();
            contextMenu->addAction(m_actionDownload);
            contextMenu->addAction(m_actionDelete_Selected);
            contextMenu->addAction(m_openFile);

            contextMenu->exec(QCursor::pos());
        }
    }

    return false;
}


void TransferHistoryItemDelegate::slotOpenFile()
{
    const QAbstractItemModel *model = static_cast <const QAbstractItemModel *> (m_selectedIndex.model());

    new KRun(model->data(m_selectedIndex, TransferHistoryCategorizedDelegate::RoleDest).toString(), m_view, true, false);
}

void TransferHistoryItemDelegate::slotDownload()
{
   const QAbstractItemModel *model = static_cast <const QAbstractItemModel *> (m_selectedIndex.model());

    NewTransferDialogHandler::showNewTransferDialog(model->data(m_selectedIndex,
                                                    TransferHistoryCategorizedDelegate::RoleUrl).toString());
}

void TransferHistoryItemDelegate::slotDeleteTransfer()
{
    const QAbstractItemModel *model = static_cast <const QAbstractItemModel *> (m_selectedIndex.model());

    emit deletedTransfer(model->data(m_selectedIndex, TransferHistoryCategorizedDelegate::RoleUrl).toString(), m_selectedIndex);
}

#include "transferhistoryitemdelegate.moc"
