/* This file is part of the KDE project

   Copyright (C) 2007 by Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "transferhistory.h"
#include "core/job.h"
#include "core/transferhistorystore.h"
#include "settings.h"
#include "ui/history/rangetreewidget.h"
#include "ui/history/transferhistorycategorizeddelegate.h"
#include "ui/history/transferhistorycategorizedview.h"
#include "ui/newtransferdialog.h"

#include "kget_debug.h"

#include <QDateTime>
#include <QDebug>
#include <QDialogButtonBox>
#include <QFile>
#include <QFileSystemWatcher>
#include <QFontDatabase>
#include <QFontMetrics>
#include <QIcon>
#include <QMenu>
#include <QModelIndex>
#include <QProgressBar>
#include <QPushButton>
#include <QStandardItem>
#include <QStandardPaths>
#include <QVariant>

#include <KIO/JobUiDelegateFactory>
#include <KIO/OpenUrlJob>
#include <KLocalizedString>
#include <KTreeWidgetSearchLine>
#include <kio/global.h>

TransferHistory::TransferHistory(QWidget *parent)
    : KGetSaveSizeDialog("TransferHistory", parent)
    , m_rangeType(TransferHistory::Date)
    , m_progressBar(new QProgressBar(this))
    , m_iconModeEnabled(true)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(i18n("Transfer History"));
    // Setup Ui-Parts from Designer
    auto *mainWidget = new QWidget(this);

    setupUi(mainWidget);

    m_view = new TransferHistoryCategorizedView(this);

    // list icon view
    auto &iconView = bt_iconview;
    auto &listView = bt_listview;

    listView->setIcon(QIcon::fromTheme("view-list-details"));
    iconView->setIcon(QIcon::fromTheme("view-list-icons"));

    connect(listView, &QAbstractButton::clicked, this, &TransferHistory::slotSetListMode);
    connect(iconView, &QAbstractButton::clicked, this, &TransferHistory::slotSetIconMode);

    clearButton->setIcon(QIcon::fromTheme("edit-clear-history"));
    actionDelete_Selected->setIcon(QIcon::fromTheme("edit-delete"));
    actionDownload->setIcon(QIcon::fromTheme("document-new"));
    m_openFile = new QAction(QIcon::fromTheme("document-open"), i18n("&Open File"), this);

    m_progressBar->setFormat(i18nc("Progress bar percent value", "%p%"));

    vboxLayout->addWidget(m_view);
    vboxLayout->addWidget(m_progressBar);

    auto *buttonBox = new QDialogButtonBox(mainWidget);
    buttonBox->clear();
    buttonBox->addButton(QDialogButtonBox::Close);
    vboxLayout->addWidget(buttonBox);

    setLayout(vboxLayout);

    watcher = new QFileSystemWatcher();
    watcher->addPath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QLatin1Char('/') + QString());
    qCDebug(KGET_DEBUG) << watcher->directories();

    m_store = TransferHistoryStore::getStore();

    connect(actionDelete_Selected, SIGNAL(triggered()), this, SLOT(slotDeleteTransfer()));
    connect(actionDownload, &QAction::triggered, this, &TransferHistory::slotDownload);
    connect(m_openFile, SIGNAL(triggered()), this, SLOT(slotOpenFile()));
    connect(clearButton, &QAbstractButton::clicked, this, &TransferHistory::slotClear);
    connect(rangeType, SIGNAL(activated(int)), this, SLOT(slotLoadRangeType(int)));
    connect(m_view, SIGNAL(deletedTransfer(QString, QModelIndex)), SLOT(slotDeleteTransfer(QString, QModelIndex)));
    connect(m_view, SIGNAL(doubleClicked(QModelIndex)), SLOT(slotOpenFile(QModelIndex)));
    connect(m_store, &TransferHistoryStore::loadFinished, this, &TransferHistory::slotLoadFinished);
    connect(m_store, &TransferHistoryStore::elementLoaded, this, &TransferHistory::slotElementLoaded);
    connect(searchBar, SIGNAL(textChanged(QString)), m_view, SLOT(setFilterRegExp(QString)));
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    slotAddTransfers();
}

TransferHistory::~TransferHistory()
{
    delete m_store;
}

QSize TransferHistory::sizeHint() const
{
    QSize sh(800, 600);
    return sh;
}

void TransferHistory::slotDeleteTransfer()
{
    if (!m_iconModeEnabled) {
        auto *range_view = qobject_cast<RangeTreeWidget *>(m_view);

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
        NewTransferDialogHandler::showNewTransferDialog(QUrl(((RangeTreeWidget *)m_view)->currentItem(0)->text()));
    }
}

void TransferHistory::contextMenuEvent(QContextMenuEvent *event)
{
    Q_UNUSED(event)

    if (!m_iconModeEnabled) {
        auto *range_view = qobject_cast<RangeTreeWidget *>(m_view);
        if (range_view->currentIndex().parent().isValid()) {
            auto *contextMenu = new QMenu(this);
            contextMenu->addAction(actionDownload);
            contextMenu->addAction(actionDelete_Selected);

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
        auto *range_view = qobject_cast<RangeTreeWidget *>(m_view);
        file = range_view->currentItem(1)->text();
    } else {
        auto *categorized_view = qobject_cast<TransferHistoryCategorizedView *>(m_view);
        file = categorized_view->data(index, TransferHistoryCategorizedDelegate::RoleDest).toString();
    }

    // qDebug() << "Try to open the file : " << file;
    if (!file.isEmpty()) {
        auto job = new KIO::OpenUrlJob(QUrl::fromLocalFile(file), this);
        job->setUiDelegate(KIO::createDefaultJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, this));
        job->start();
    }
}

void TransferHistory::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event)
    disconnect(watcher, &QFileSystemWatcher::directoryChanged, this, &TransferHistory::slotAddTransfers); // Prevent reloading of TransferHistory when saving
    deleteLater();
}

QString TransferHistory::statusText(int status) const
{
    switch (status) {
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
        auto *cat_view = qobject_cast<TransferHistoryCategorizedView *>(m_view);
        cat_view->clear();
        switch (type) {
        case TransferHistory::Size:
            cat_view->setCategorizedDelegate(new SizeCategorizedDelegate());
            break;
        case TransferHistory::Date:
            cat_view->setCategorizedDelegate(new DateCategorizedDelegate());
            break;
        default:
            cat_view->setCategorizedDelegate(new HostCategorizedDelegate());
        }
    } else {
        auto *range_view = qobject_cast<RangeTreeWidget *>(m_view);
        auto *font = new QFontMetrics(QFontDatabase::systemFont(QFontDatabase::GeneralFont));
        range_view->clear();

        range_view->setLabels(QStringList() << i18n("Source File") << i18n("Destination") << i18n("Time") << i18n("File Size") << i18n("Status"));
        range_view->setRangeDelegate(nullptr);

        switch (m_rangeType) {
        case TransferHistory::Host:
            range_view->setRangeDelegate(new HostRangeDelegate(this));
            break;
        case TransferHistory::Size:
            range_view->addRange(QVariant(0), QVariant(1024 * 1024), i18n("Less than 1MiB"));
            range_view->addRange(QVariant(1024 * 1024), QVariant(1024 * 1024 * 10), i18n("Between 1MiB-10MiB"));
            range_view->addRange(QVariant(1024 * 1024 * 10), QVariant(1024 * 1024 * 100), i18n("Between 10MiB-100MiB"));
            range_view->addRange(QVariant(1024 * 1024 * 100), QVariant(1024 * 1024 * 1024), i18n("Between 100MiB-1GiB"));
            range_view->addRange(QVariant((double)1024 * 1024 * 1024), QVariant((double)1024 * 1024 * 1024 * 10), i18n("More than 1GiB"));
            break;
        default:
            range_view->addRange(QVariant(0), QVariant(1), i18n("Today"));
            range_view->addRange(QVariant(1), QVariant(7), i18n("Last week"));
            range_view->addRange(QVariant(7), QVariant(30), i18n("Last month"));
            range_view->addRange(QVariant(30), QVariant(-1), i18n("A long time ago"));
        }

        QList<int> list = Settings::historyColumnWidths();

        if (!list.isEmpty()) {
            int j = 0;
            foreach (int i, list) {
                range_view->setColumnWidth(j, i);
                j++;
            }
        } else {
            range_view->setColumnWidth(0, 200);
            range_view->setColumnWidth(1, 250);
            range_view->setColumnWidth(2, font->horizontalAdvance(QDate::currentDate().toString()));
            range_view->setColumnWidth(3, font->horizontalAdvance("1500000 KiB"));
            range_view->setColumnWidth(4, font->horizontalAdvance(i18nc("the transfer has been finished", "Finished")));
        }
    }

    slotAddTransfers();
}

void TransferHistory::slotSetListMode()
{
    m_iconModeEnabled = false;
    delete m_view;
    m_view = new RangeTreeWidget(this);
    vboxLayout->insertWidget(1, m_view);
    slotLoadRangeType(m_rangeType);

    connect(searchBar, SIGNAL(textChanged(QString)), m_view, SLOT(setFilterRegExp(QString)));
    // we connect the doubleClicked signal over an item to the open file action
    connect(m_view, SIGNAL(doubleClicked(QModelIndex)), SLOT(slotOpenFile(QModelIndex)));
}

void TransferHistory::slotSetIconMode()
{
    m_iconModeEnabled = true;
    delete m_view;
    m_view = new TransferHistoryCategorizedView(this);
    vboxLayout->insertWidget(1, m_view);
    slotLoadRangeType(m_rangeType);

    connect(searchBar, SIGNAL(textChanged(QString)), m_view, SLOT(setFilterRegExp(QString)));
    connect(m_view, SIGNAL(deletedTransfer(QString, QModelIndex)), SLOT(slotDeleteTransfer(QString, QModelIndex)));
    connect(m_view, SIGNAL(doubleClicked(QModelIndex)), SLOT(slotOpenFile(QModelIndex)));
}

void TransferHistory::slotElementLoaded(int number, int total, const TransferHistoryItem &item)
{
    m_progressBar->setValue(number * 100 / total);

    if (m_iconModeEnabled) {
        ((TransferHistoryCategorizedView *)m_view)->addData(item.dateTime().date(), item.source(), item.dest(), item.size());
    } else {
        QVariantList attributeList;
        attributeList.append(item.source());
        attributeList.append(item.dest());
        attributeList.append(item.dateTime().date().toString());
        attributeList.append(KIO::convertSize(item.size()));
        attributeList.append(statusText(item.state()));

        QVariant data;
        if (m_rangeType == TransferHistory::Date) {
            QDate date = item.dateTime().date();
            data = QVariant(date.daysTo(QDate::currentDate()));
        } else if (m_rangeType == TransferHistory::Host) {
            data = QVariant(item.source());
        } else {
            data = QVariant(item.size());
        }
        ((RangeTreeWidget *)m_view)->add(data, attributeList);
    }
}

void TransferHistory::slotLoadFinished()
{
    m_progressBar->hide();
}

#include "moc_transferhistory.cpp"
