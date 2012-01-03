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
#include <QVariant>

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
    : KGetSaveSizeDialog("TransferHistory", parent),
    m_rangeType(TransferHistory::Date),
    m_progressBar(new QProgressBar(this)),
    m_iconModeEnabled(true)
{
    setAttribute(Qt::WA_DeleteOnClose);
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
    m_openFile = new QAction(KIcon("document-open"), i18n("&Open File"), this);
    setMainWidget(mainWidget);

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
    connect(m_view, SIGNAL(deletedTransfer(QString,QModelIndex)),
                    SLOT(slotDeleteTransfer(QString,QModelIndex)));
    connect(m_view, SIGNAL(doubleClicked(QModelIndex)), SLOT(slotOpenFile(QModelIndex)));
    connect(m_store, SIGNAL(loadFinished()), SLOT(slotLoadFinished()));
    connect(m_store, SIGNAL(elementLoaded(int,int,TransferHistoryItem)),
                     SLOT(slotElementLoaded(int,int,TransferHistoryItem)));
    connect(m_searchBar, SIGNAL(textChanged(QString)), m_view, SLOT(setFilterRegExp(QString)));
    slotAddTransfers();
}

TransferHistory::~TransferHistory()
{
    delete m_store;
}

QSize TransferHistory::sizeHint() const
{
    QSize sh(800,600);
    return sh;
}

void TransferHistory::slotDeleteTransfer()
{
    if (!m_iconModeEnabled) {
        RangeTreeWidget *range_view = qobject_cast <RangeTreeWidget *> (m_view);

        slotDeleteTransfer(range_view->currentItem(0)->text());

        slotLoadRangeType(m_rangeType);
    }
}

void TransferHistory::slotDeleteTransfer(const QString &transferName, const QModelIndex &index)
{
    Q_UNUSED(index)

    TransferHistoryItem item;
    item.setSource(transferName);
    m_store->deleteItem(item);

    slotLoadRangeType(m_rangeType);
}

void TransferHistory::slotAddTransfers()
{
    m_progressBar->show();
    m_store->load();
}

void TransferHistory::slotClear()
{
    m_store->clear();

    slotLoadRangeType(m_rangeType);
}

void TransferHistory::slotWriteDefault()
{
    // not neded ??
}

void TransferHistory::slotDownload()
{
    if (!m_iconModeEnabled) {
        NewTransferDialogHandler::showNewTransferDialog(((RangeTreeWidget *) m_view)->currentItem(0)->text());
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

void TransferHistory::slotOpenFile(const QModelIndex &index)
{
    QString file;

    if (!m_iconModeEnabled) {
        RangeTreeWidget *range_view = qobject_cast <RangeTreeWidget *> (m_view);
        file = range_view->currentItem(1)->text();
    }
    else {
        TransferHistoryCategorizedView *categorized_view = qobject_cast <TransferHistoryCategorizedView *> (m_view);
        file = categorized_view->data(index, TransferHistoryCategorizedDelegate::RoleDest).toString();
    }

    kDebug() << "Try to open the file : " << file;
    if (!file.isEmpty()) {
        new KRun(file, this, true, false);
    }
}

void TransferHistory::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event)
    disconnect(watcher, SIGNAL(directoryChanged(QString)), this, SLOT(slotAddTransfers()));//Prevent reloading of TransferHistory when saving
    deleteLater();
}

QString TransferHistory::statusText(int status) const
{
    switch (status)
    {
        case Job::Running:
            return i18nc("The transfer is running", "Running");
        case Job::Stopped:
            return i18nc("The transfer is stopped", "Stopped");
        case Job::Aborted:
            return i18nc("The transfer is aborted", "Aborted");
        case Job::Finished:
            return i18nc("The transfer is finished", "Finished");
        default:
            return QString();
    }
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
    } else {
        RangeTreeWidget *range_view = qobject_cast <RangeTreeWidget *> (m_view);
        QFontMetrics *font = new QFontMetrics(KGlobalSettings::generalFont());
        range_view->clear();

        range_view->setLabels(QStringList() << i18n("Source File") << i18n("Destination") << i18n("Time") << i18n("File Size") << i18n("Status"));
        range_view->setRangeDelegate(0);

        switch(m_rangeType)
        {
            case TransferHistory::Host :
                range_view->setRangeDelegate(new HostRangeDelegate(this));
                break;
            case TransferHistory::Size :
                range_view->addRange(QVariant(0), QVariant(1024 * 1024),
                                                  i18n("Less than 1MiB"));
                range_view->addRange(QVariant(1024 * 1024), QVariant(1024 * 1024 * 10),
                                                  i18n("Between 1MiB-10MiB"));
                range_view->addRange(QVariant(1024 * 1024 * 10), QVariant(1024 * 1024 * 100),
                                                  i18n("Between 10MiB-100MiB"));
                range_view->addRange(QVariant(1024 * 1024 * 100), QVariant(1024 * 1024 *1024),
                                                  i18n("Between 100MiB-1GiB"));
                range_view->addRange(QVariant((double) 1024 * 1024 * 1024), QVariant((double) 1024 * 1024 * 1024 * 10),
                                                  i18n("More than 1GiB"));
                break;
            default:
                range_view->addRange(QVariant(0), QVariant(1), i18n("Today"));
                range_view->addRange(QVariant(1), QVariant(7), i18n("Last week"));
                range_view->addRange(QVariant(7), QVariant(30), i18n("Last month"));
                range_view->addRange(QVariant(30), QVariant(-1), i18n("A long time ago"));
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

    connect(m_searchBar, SIGNAL(textChanged(QString)), m_view, SLOT(setFilterRegExp(QString)));
    // we connect the doubleClicked signal over an item to the open file action
    connect(m_view, SIGNAL(doubleClicked(QModelIndex)), SLOT(slotOpenFile(QModelIndex)));
}

void TransferHistory::slotSetIconMode()
{
    m_iconModeEnabled = true;
    delete m_view;
    m_view = new TransferHistoryCategorizedView(this);
    m_verticalLayout->insertWidget(1, m_view);
    slotLoadRangeType(m_rangeType);

    connect(m_searchBar, SIGNAL(textChanged(QString)), m_view, SLOT(setFilterRegExp(QString)));
    connect(m_view, SIGNAL(deletedTransfer(QString,QModelIndex)),
                    SLOT(slotDeleteTransfer(QString,QModelIndex)));
    connect(m_view, SIGNAL(doubleClicked(QModelIndex)), SLOT(slotOpenFile(QModelIndex)));
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
        attributeList.append(statusText(item.state()));

        QVariant data;
        if(m_rangeType == TransferHistory::Date) {
            QDate date = item.dateTime().date();
            data = QVariant(date.daysTo(QDate::currentDate()));
        }
        else if(m_rangeType == TransferHistory::Host) {
            data = QVariant(item.source());
        }
        else {
            data = QVariant(item.size());
        }
        ((RangeTreeWidget *) m_view)->add(data, attributeList);
    }
}

void TransferHistory::slotLoadFinished()
{
    m_progressBar->hide();
}
