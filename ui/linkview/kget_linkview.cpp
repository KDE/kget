/* This file is part of the KDE project

   Copyright (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>
   Copyright (C) 2007 Urs Wolfer <uwolfer @ kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "kget_linkview.h"
#include "core/kget.h"
#include "core/linkimporter.h"
#include "ui/newtransferdialog.h"

#include <KActionCollection>
#include <KDebug>
#include <KShortcut>
#include <kstandardaction.h>
#include <KUrlRequester>
#include <kiconloader.h>
#include <kicon.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <KLineEdit>
#include <KMimeType>

#include <QApplication>
#include <QClipboard>
#include <QProcess>
#include <QTreeView>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QButtonGroup>
#include <QPushButton>
#include <QProgressBar>
#include <QCheckBox>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QHeaderView>

KGetLinkView::KGetLinkView(QWidget *parent)
  : KDialog(parent),
    m_showWebContent(false)
{
    setPlainCaption(i18n("KGet"));

    // proxy model to filter links
    m_proxyModel = new QSortFilterProxyModel();
    m_proxyModel->setDynamicSortFilter(true);

    m_treeWidget = new QTreeView(this);
    m_treeWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_treeWidget->setModel(m_proxyModel);
    m_treeWidget->setSelectionBehavior(QAbstractItemView::SelectItems);
    m_treeWidget->setSelectionMode(QAbstractItemView::NoSelection);
    m_treeWidget->setAlternatingRowColors(true);
    m_treeWidget->setRootIsDecorated(false);
    m_treeWidget->setSortingEnabled(true);
    m_treeWidget->setAllColumnsShowFocus(true);
    m_treeWidget->setColumnWidth(0, 200); // make the filename column bigger by default

    connect(m_treeWidget, SIGNAL(doubleClicked(const QModelIndex &)), 
                        this, SLOT(uncheckItem(const QModelIndex &)));

    KLineEdit *searchLine = new KLineEdit(this);
    searchLine->setClearButtonShown(true);
    searchLine->setClickMessage(i18n("Filter files here..."));
    connect(searchLine, SIGNAL(textChanged(QString)), SLOT(updateSelectAllText(QString)));

    setButtons(KDialog::None);

    filterButtonsGroup = new QButtonGroup(this);
    filterButtonsGroup->setExclusive(true);
    connect(filterButtonsGroup, SIGNAL(buttonClicked(int)), SLOT(doFilter(int)));

    // filter buttons and filter line text box
    QHBoxLayout *filterLayout = new QHBoxLayout;
    filterLayout->addWidget(new QLabel(i18n("Show:")));

    for (uint i = 0; i < sizeof(filters) / sizeof(*filters); ++i) {
        filterLayout->addWidget(createFilterButton(filters[i].icon, filters[i].name,
                                filterButtonsGroup, filters[i].type, filters[i].defaultFilter));
    }

    filterLayout->addWidget(searchLine);

    // import url layout
    QPushButton *importButton = new QPushButton("Import links", this);
    m_urlRequester = new KUrlRequester(this);
    m_importerLayout = new QHBoxLayout;
    m_importerLayout->addWidget(m_urlRequester);
    m_importerLayout->addWidget(importButton);

    m_linkImporter = 0;

    // import progressbar
    m_progressBar = new QProgressBar(this);
    m_progressBar->hide();

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setMargin(0);
    mainLayout->addLayout(m_importerLayout);
    mainLayout->addLayout(filterLayout);
    mainLayout->addWidget(m_treeWidget);
    mainLayout->addWidget(m_progressBar);

    // Bottoms buttons
    QHBoxLayout *bottomButtonsLayout = new QHBoxLayout;

    checkAllButton = new QPushButton(i18n("Select all"));
    QCheckBox *showWebContentButton = new QCheckBox(i18n("Show web content"));
    downloadCheckedButton = new QPushButton( KIcon("kget"), i18n("Download checked"));
    downloadCheckedButton->setEnabled(false);
    QPushButton *cancelButton = new QPushButton(KIcon("dialog-cancel"), i18n("Cancel"));

    connect(cancelButton, SIGNAL(clicked()), this, SLOT(hide()));
    connect(checkAllButton, SIGNAL(clicked()), this, SLOT(checkAll()));
    connect(downloadCheckedButton, SIGNAL(clicked()), this, SLOT(slotStartLeech()));
    connect(showWebContentButton, SIGNAL(stateChanged(int)), this, SLOT(slotShowWebContent(int)));
    connect(importButton, SIGNAL(clicked()), this, SLOT(slotStartImport()));

    bottomButtonsLayout->addWidget(checkAllButton);
    bottomButtonsLayout->addWidget(showWebContentButton);
    bottomButtonsLayout->addWidget(downloadCheckedButton);
    bottomButtonsLayout->addWidget(cancelButton);

    mainLayout->addLayout(bottomButtonsLayout);

    QWidget *mainWidget = new QWidget(this);
    mainWidget->setLayout(mainLayout);
    setMainWidget(mainWidget);

    resize(600, 300);
}

KGetLinkView::~KGetLinkView()
{
    delete(m_linkImporter);
}

void KGetLinkView::setLinks(const QList <QString> &links)
{
    m_links = QList <QString> ();
    foreach(QString link, links) {
        m_links << link;
    }

    showLinks(m_links);
}

void KGetLinkView::showLinks( const QList<QString>& links )
{
    QStandardItemModel *model = new QStandardItemModel(0, 5, this);

    model->setHeaderData(0, Qt::Horizontal, i18n("Auxiliar header"));
    model->setHeaderData(1, Qt::Horizontal, i18n("File Name"));
    model->setHeaderData(2, Qt::Horizontal, i18n("Description"));
    model->setHeaderData(3, Qt::Horizontal, i18nc("list header: type of file", "File Type"));
    model->setHeaderData(4, Qt::Horizontal, i18n("Location (URL)"));

    foreach (const QString &linkitem, links) {
        KUrl url(linkitem);
        QString file = url.fileName();
        if (file.isEmpty())
            file = QString(url.host());

        KMimeType::Ptr mt = KMimeType::findByUrl(linkitem, 0, true, true);

        QList<QStandardItem*> items;

        QStandardItem *item = new QStandardItem(file);
        item->setIcon(KIcon(mt->iconName()));
        item->setCheckable(true);
        item->setData(QVariant(url.prettyUrl()), Qt::EditRole);

        items << new QStandardItem(QString::number(model->rowCount()));
        items << item;
        items << new QStandardItem();
        items << new QStandardItem(mt->comment());
        items << new QStandardItem(url.prettyUrl());

        model->insertRow(model->rowCount(), items);
    }

    connect(model, SIGNAL(itemChanged(QStandardItem *)), this, SLOT(selectionChanged()));
    m_proxyModel->setSourceModel(model);
    m_proxyModel->setFilterKeyColumn(1);

    m_treeWidget->header()->hideSection(0);

    slotShowWebContent(false);
}

void KGetLinkView::slotStartLeech()
{
    QStandardItemModel *model = (QStandardItemModel*) m_proxyModel->sourceModel();
    KUrl::List urls;

    for(int row=0;row<model->rowCount();row++) {
        QStandardItem *checkeableItem = model->item(row, 1);

        if(checkeableItem->checkState() == Qt::Checked) {
            urls.append(KUrl(model->data(model->index(row, 1), Qt::EditRole).toString()));
        }
    }

    NewTransferDialog::showNewTransferDialog(urls);
    accept(); // close the dialog
}

void KGetLinkView::setPageUrl( const QString& url )
{
    setPlainCaption( i18n( "Links in: %1 - KGet", url ) );
}

void KGetLinkView::importUrl(const QString &url)
{
    if(url.isEmpty()) {
        KUrl clipboardUrl = KUrl(QApplication::clipboard()->text(QClipboard::Clipboard).trimmed());
        if(clipboardUrl.isValid()) {
            m_urlRequester->setUrl(clipboardUrl);
        }
    }
    else {
        m_urlRequester->setUrl(KUrl(url));
        slotStartImport();
    }
}

void KGetLinkView::selectionChanged()
{
    bool buttonEnabled = false;
    QStandardItemModel *model = (QStandardItemModel*) m_proxyModel->sourceModel();

    for(int row=0;row<model->rowCount();row++) {
        QStandardItem *checkeableItem = model->item(row, 1);

        if(checkeableItem->checkState() == Qt::Checked) {
            buttonEnabled = true;
        }
    }

    downloadCheckedButton->setEnabled(buttonEnabled);
}

void KGetLinkView::updateSelectAllText(const QString &text)
{
    doFilter(filterButtonsGroup->checkedId(), text);
}

void KGetLinkView::doFilter(int id, const QString &textFilter)
{
    QString filter;
    switch(id) {
        case KGetLinkView::AudioFiles:
            filter = AUDIO_FILES_REGEXP;
            break;
        case KGetLinkView::VideoFiles:
            filter = VIDEO_FILES_REGEXP;
            break;
        case KGetLinkView::CompressedFiles:
            filter = COMPRESSED_FILES_REGEXP;
            break;
        case KGetLinkView::NoFilter:
        default:
            filter =  m_showWebContent ? QString() : WEB_CONTENT_REGEXP;
    }

    if(!textFilter.isEmpty()) {
        if(filter.isEmpty()) {
            filter = textFilter;
        }
        else {
            filter.replace(".(", "" + textFilter + "*(");
        }
    }

    checkAllButton->setText((textFilter.isEmpty() && id == KGetLinkView::NoFilter) 
                    ? i18n("Select all") : i18n("Select all filtered"));
    m_proxyModel->setFilterRegExp(QRegExp(filter, Qt::CaseInsensitive));
}

void KGetLinkView::checkAll()
{
    QStandardItemModel *itemsModel = (QStandardItemModel *) m_proxyModel->sourceModel();
    for(int row=0;row<m_proxyModel->rowCount();row++) {
        QModelIndex index = m_proxyModel->mapToSource(m_proxyModel->index(row, 3));
        QStandardItem *item = itemsModel->item(index.row(), 1);
        item->setCheckState(Qt::Checked);
    }
}

void KGetLinkView::uncheckItem(const QModelIndex &index)
{
    QStandardItemModel *model = (QStandardItemModel*) m_proxyModel->sourceModel();
    if(index.column() != 0) {
        QStandardItem *item = model->itemFromIndex(model->index(m_proxyModel->mapToSource(index).row(), 1));
        item->setCheckState(item->checkState() == Qt::Checked ? Qt::Unchecked : Qt::Checked);
    }
}

void KGetLinkView::slotStartImport()
{
    if(m_linkImporter) {
        delete(m_linkImporter);
    }
    m_linkImporter = new LinkImporter(m_urlRequester->url(), this);

    connect(m_linkImporter, SIGNAL(progress(int)), SLOT(slotImportProgress(int)));
    connect(m_linkImporter, SIGNAL(finished()), SLOT(slotImportFinished()));

    if(!m_urlRequester->url().isLocalFile()) {
        m_linkImporter->copyRemoteFile();
    }

    m_linkImporter->start();
    m_progressBar->show();
}

void KGetLinkView::slotImportProgress(int progress)
{
    m_progressBar->setValue(progress);
}

void KGetLinkView::slotImportFinished()
{
    m_progressBar->hide();
    m_links = QList <QString> (m_linkImporter->links());
    showLinks(m_links);
}

QAbstractButton *KGetLinkView::createFilterButton(const QString &icon, const QString &description,
                            QButtonGroup *group, uint filterType, bool checked)
{
    QPushButton *filterButton = new QPushButton(KIcon(icon), description);
    filterButton->setCheckable(true);
    filterButton->setChecked(checked);

    group->addButton(filterButton, filterType);

    return filterButton;
}

void KGetLinkView::slotShowWebContent(int mode)
{
    m_showWebContent = (mode == Qt::Checked);
    doFilter(filterButtonsGroup->checkedId());
}

#include "kget_linkview.moc"
