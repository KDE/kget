/* This file is part of the KDE project

   Copyright (C) 2007 by Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "transferhistory.h"
#include "ui/newtransferdialog.h"
#include "settings.h"
#include "ui/history/transferhistorycategorizedview.h"
#include "ui/history/transferhistorycategorizeddelegate.h"
#include "ui/history/rangetreewidget.h"
#include "core/transferhistorystore.h"
#include "core/job.h"

#include <QDateTime>
#include <QFile>
#include <QFileSystemWatcher>
#include <QFontMetrics>
#include <QLineEdit>
#include <QMenu>
#include <QModelIndex>
#include <QProgressBar>
#include <QStandardItem>

#include <KDebug>
#include <KStandardDirs>
#include <KMessageBox>
#include <KToolBar>
#include <KTreeWidgetSearchLine>
#include <KRun>
#include <KGlobalSettings>
#include <kio/global.h>
#include <KPushButton>
#include <KIcon>

TransferHistory::TransferHistory(QWidget *parent)
    : KDialog(parent),
    m_rangeType(TransferHistory::Date),
    m_progressBar(new QProgressBar(this)),
    m_iconModeEnabled(true)
{
    setCaption(i18n("Transfer History"));
    setButtons(KDialog::Close);
    //Setup Ui-Parts from Designer
    QWidget *mainWidget = new QWidget(this);

    Ui::TransferHistory widget;
    widget.setupUi(mainWidget);

    m_view = new TransferHistoryCategorizedView(this);

    // list icon view
    m_iconView = widget.bt_iconview;
    m_listView = widget.bt_listview;

    m_listView->setIcon(KIcon("view-list-details"));
    m_iconView->setIcon(KIcon("view-list-icons"));

    connect(m_listView, SIGNAL(clicked()), SLOT(slotSetListMode()));
    connect(m_iconView, SIGNAL(clicked()), SLOT(slotSetIconMode()));

    // range type
    m_rangeTypeCombobox = widget.rangeType;

    m_verticalLayout = widget.vboxLayout;
    m_hboxLayout = widget.hboxLayout;
    m_searchBar = widget.searchBar;
    //m_searchBar->setTreeWidget(m_treeWidget);
    m_clearButton = widget.clearButton;
    m_clearButton->setIcon(KIcon("edit-clear-history"));
    m_actionDelete_Selected = widget.actionDelete_Selected;
    m_actionDelete_Selected->setIcon(KIcon("edit-delete"));
    m_actionDownload = widget.actionDownload;
    m_actionDownload->setIcon(KIcon("document-new"));
    m_openFile = new QAction(KIcon("document-open"), "&Open File", this);
    setMainWidget(mainWidget);
    setInitialSize(QSize(800, 400));

    m_verticalLayout->addWidget(m_view);
    m_verticalLayout->addWidget(m_progressBar);

    watcher = new QFileSystemWatcher();
    watcher->addPath(KStandardDirs::locateLocal("appdata", QString()));
    kDebug(5001) << watcher->directories();

    m_store = TransferHistoryStore::getStore();

    connect(m_actionDelete_Selected, SIGNAL(triggered()), this, SLOT(slotDeleteTransfer()));
    connect(m_actionDownload, SIGNAL(triggered()), this, SLOT(slotDownload()));
    connect(m_openFile, SIGNAL(triggered()), this, SLOT(slotOpenFile()));
    connect(m_clearButton, SIGNAL(clicked()), this, SLOT(slotClear()));
    connect(m_rangeTypeCombobox, SIGNAL(activated(int)), this, SLOT(slotLoadRangeType(int)));
    connect(m_searchBar, SIGNAL(textChanged(const QString &)), m_view, SLOT(setFilterRegExp(const QString &)));
    connect(m_view, SIGNAL(deletedTransfer(const QString &, const QModelIndex &)),
                    SLOT(slotDeleteTransfer(const QString &, const QModelIndex &)));
    connect(m_store, SIGNAL(loadFinished()), SLOT(slotLoadFinished()));
    connect(m_store, SIGNAL(elementLoaded(int, int, const TransferHistoryItem &)),
                     SLOT(slotElementLoaded(int, int, const TransferHistoryItem &)));
    slotAddTransfers();
}

TransferHistory::~TransferHistory()
{
    delete m_store;
}

void TransferHistory::slotDeleteTransfer()
{
    if (!m_iconModeEnabled) {
        RangeTreeWidget *range_view = qobject_cast <RangeTreeWidget *> (m_view);

        slotDeleteTransfer(range_view->currentItem(0)->text());

        range_view->removeRow(range_view->currentItem(0)->index().row(),
                                range_view->currentItem(0)->index().parent());
    }
}

void TransferHistory::slotDeleteTransfer(const QString &transferName, const QModelIndex &index)
{
    TransferHistoryItem item;
    item.setSource(transferName);

    m_store->deleteItem(item);

    if (m_iconModeEnabled && index.isValid()) {
        TransferHistoryCategorizedView *view = qobject_cast <TransferHistoryCategorizedView *> (m_view);

        view->removeRow(index.row(), index.parent());
    }
}

void TransferHistory::slotAddTransfers()
{
    m_progressBar->show();
    m_store->load();
}

void TransferHistory::slotClear()
{
    // TODO
}

void TransferHistory::slotWriteDefault()
{
    // not neded ??
}

void TransferHistory::slotDownload()
{
    if (!m_iconModeEnabled) {
        NewTransferDialog::showNewTransferDialog(((RangeTreeWidget *) m_view)->currentItem(0)->text());
    }
}

void TransferHistory::contextMenuEvent(QContextMenuEvent *event)
{
    Q_UNUSED(event)

    if (!m_iconModeEnabled) {
        RangeTreeWidget *range_view = qobject_cast <RangeTreeWidget *> (m_view);
        if(range_view->currentIndex().parent().isValid()) {
            QMenu *contextMenu = new QMenu(this);
            contextMenu->addAction(m_actionDownload);
            contextMenu->addAction(m_actionDelete_Selected);

            if (range_view->currentItem(4)->data().toInt() == Job::Finished)
                contextMenu->addAction(m_openFile);
            contextMenu->exec(QCursor::pos());
        }
    }
}

void TransferHistory::slotOpenFile()
{
    if (!m_iconModeEnabled) {
        RangeTreeWidget *range_view = qobject_cast <RangeTreeWidget *> (m_view);
        new KRun(range_view->currentItem(1)->text(), this, true, false);
    }
}

void TransferHistory::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event);
    disconnect(watcher, SIGNAL(directoryChanged(const QString &)), this, SLOT(slotAddTransfers()));//Prevent reloading of TransferHistory when saving
    deleteLater();
}

void TransferHistory::slotLoadRangeType(int type)
{
    m_rangeType = type;
    if (m_iconModeEnabled) {
        TransferHistoryCategorizedView *cat_view = qobject_cast <TransferHistoryCategorizedView *> (m_view);
        cat_view->clear();
        switch(type)
        {
            case TransferHistory::Size :
                cat_view->setCategorizedDelegate(new SizeCategorizedDelegate());
                break;
            case TransferHistory::Date :
                cat_view->setCategorizedDelegate(new DateCategorizedDelegate());
                break;
            default:
                cat_view->setCategorizedDelegate(new HostCategorizedDelegate());
        }
    }
    else {
        RangeTreeWidget *range_view = qobject_cast <RangeTreeWidget *> (m_view);
        QFontMetrics *font = new QFontMetrics(KGlobalSettings::generalFont());
        range_view->clear();

        range_view->setLabels(QStringList() << i18n("Source-File") << i18n("Destination") << i18n("Time") << i18n("File Size") << i18n("Status"));

        switch(m_rangeType)
        {
            case TransferHistory::Size :
                range_view->addRange(0, 1024 * 1024, i18n("Less than 1MiB"));
                range_view->addRange(1024 * 1024, 1024 * 1024 * 10, i18n("Between 1MiB-10MiB"));
                range_view->addRange(1024 * 1024 * 10, 1024 * 1024 * 100, i18n("Between 10MiB-100MiB"));
                range_view->addRange(1024 * 1024 * 100, 1024 * 1024 *1024, i18n("Between 100MiB-1GiB"));
                range_view->addRange((long) 1024 * 1024 * 1024, (long) 1024 * 1024 * 1024 * 10, i18n("More than 1GiB"));
                // TODO : Fix that integer overflow..
                break;
            default:
                range_view->addRange(0, 1, i18n("Today"));
                range_view->addRange(1, 7, i18n("Last week"));
                range_view->addRange(7, 30, i18n("Last month"));
                range_view->addRange(30, -1, i18n("A long time ago"));
        }

        QList<int> list = Settings::historyColumnWidths();

        if (!list.isEmpty())
        {
            int j = 0;
            foreach (int i, list)
            {
                range_view->setColumnWidth(j, i);
                j++;
            }
        }
        else
        {
            range_view->setColumnWidth(0, 200);
            range_view->setColumnWidth(1, 250);
            range_view->setColumnWidth(2, font->width(QDate::currentDate().toString()));
            range_view->setColumnWidth(3, font->width("1500000 KiB"));
            range_view->setColumnWidth(4, font->width(i18nc("the transfer has been finished", "Finished")));
        }
    }

    slotAddTransfers();
}

void TransferHistory::slotSetListMode()
{
    m_iconModeEnabled = false;
    delete m_view;
    m_view = new RangeTreeWidget(this);
    m_verticalLayout->insertWidget(1, m_view);
    slotLoadRangeType(m_rangeType);
}

void TransferHistory::slotSetIconMode()
{
    m_iconModeEnabled = true;
    delete m_view;
    m_view = new TransferHistoryCategorizedView(this);
    m_verticalLayout->insertWidget(1, m_view);
    slotLoadRangeType(m_rangeType);
}

void TransferHistory::slotElementLoaded(int number, int total, const TransferHistoryItem &item)
{
    m_progressBar->setValue(number*100/total);

    if (m_iconModeEnabled) {
            ((TransferHistoryCategorizedView *) m_view)->addData(
                    item.dateTime().date(), item.source(), item.dest(), item.size());
    }
    else {
        QVariantList attributeList;
        attributeList.append(item.source());
        attributeList.append(item.dest());
        attributeList.append(item.dateTime().date().toString());
        attributeList.append(KIO::convertSize(item.size()));
        attributeList.append(item.state());

        int data = 0;
        if(m_rangeType == TransferHistory::Date) {
            QDate date = item.dateTime().date();
            data = date.daysTo(QDate::currentDate());
        }
        else {
            data = item.size();
        }
        ((RangeTreeWidget *) m_view)->add(data, attributeList);
    }
}

void TransferHistory::slotLoadFinished()
{
    m_progressBar->hide();
}
