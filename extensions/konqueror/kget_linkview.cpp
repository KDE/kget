/* This file is part of the KDE project

   Copyright (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>
   Copyright (C) 2007 Urs Wolfer <uwolfer @ kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "kget_linkview.h"
#include "kget_interface.h"

#include <KActionCollection>
#include <KShortcut>
#include <kstandardaction.h>
#include <kiconloader.h>
#include <kicon.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <KLineEdit>

#include <QProcess>
#include <QTreeView>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QButtonGroup>
#include <QPushButton>
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

    const filterDefinition *filter = filters;
    while(!filter->icon.isEmpty()) {
        filterLayout->addWidget(createFilterButton(filter->icon, filter->name,
                        filterButtonsGroup, filter->type, filter->defaultFilter));
        ++filter;
    }

    filterLayout->addWidget(searchLine);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setMargin(0);
    mainLayout->addLayout(filterLayout);
    mainLayout->addWidget(m_treeWidget);

    // Bottoms buttons
    QHBoxLayout *bottomButtonsLayout = new QHBoxLayout(this);

    checkAllButton = new QPushButton(i18n("Select all"));
    QCheckBox *showWebContentButton = new QCheckBox(i18n("Show web content"));
    downloadCheckedButton = new QPushButton( KIcon("kget"), i18n("Download checked"));
    downloadCheckedButton->setEnabled(false);
    QPushButton *cancelButton = new QPushButton(KIcon("dialog-cancel"), i18n("Cancel"));

    connect(cancelButton, SIGNAL(clicked()), this, SLOT(hide()));
    connect(checkAllButton, SIGNAL(clicked()), this, SLOT(checkAll()));
    connect(downloadCheckedButton, SIGNAL(clicked()), this, SLOT(slotStartLeech()));
    connect(showWebContentButton, SIGNAL(stateChanged(int)), this, SLOT(slotShowWebContent(int)));

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
    qDeleteAll(m_links);
}

void KGetLinkView::setLinks( QList<LinkItem*>& links )
{
    m_links = links; // now we 0wn them
    showLinks( m_links );
}

void KGetLinkView::showLinks( const QList<LinkItem*>& links )
{
    QStandardItemModel *model = new QStandardItemModel(0, 5, this);

    model->setHeaderData(0, Qt::Horizontal, i18n("Auxiliar header"));
    model->setHeaderData(1, Qt::Horizontal, i18n("File Name"));
    model->setHeaderData(2, Qt::Horizontal, i18n("Description"));
    model->setHeaderData(3, Qt::Horizontal, i18nc("list header: type of file", "File Type"));
    model->setHeaderData(4, Qt::Horizontal, i18n("Location (URL)"));

    foreach (LinkItem* linkitem, links) {
        QString file = linkitem->url.fileName();
        if ( file.isEmpty() )
            file = QString(linkitem->url.host());

        QList<QStandardItem*> items;

        QStandardItem *item = new QStandardItem(file);
        item->setIcon(KIcon(linkitem->icon));
        item->setCheckable(true);

        items << new QStandardItem(QString::number(model->rowCount()));
        items << item;
        items << new QStandardItem(linkitem->text);
        items << new QStandardItem(linkitem->mimeType);
        items << new QStandardItem(linkitem->url.prettyUrl());

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
    QStringList urls;

    for(int row=0;row<model->rowCount();row++) {
        QStandardItem *checkeableItem = model->item(row, 1);

        if(checkeableItem->checkState() == Qt::Checked) {
            urls.append(model->data(model->index(row, 4)).toString());
        }
    }

    if(!QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.kget"))
    {
        QProcess *kgetProcess = new QProcess(this);
        urls << "--startWithoutAnimation" ;
        kgetProcess->startDetached("kget", urls);
    }
    else
    {
        OrgKdeKgetInterface kgetInterface("org.kde.kget", "/KGet", QDBusConnection::sessionBus());
        kgetInterface.addTransfers(urls.join(";"), QString(), true);
    }

    accept(); // close the dialog
}

void KGetLinkView::setPageUrl( const QString& url )
{
    setPlainCaption( i18n( "Links in: %1 - KGet", url ) );
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
