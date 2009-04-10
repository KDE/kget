/* This file is part of the KDE project

   Copyright (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>
   Copyright (C) 2007 Urs Wolfer <uwolfer @ kde.org>
   Copyright (C) 2009 Matthias Fuchs <mat69@gmx.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "kget_linkview.h"
#include "kget_sortfilterproxymodel.h"
#include "core/kget.h"
#include "core/linkimporter.h"
#include "ui/newtransferdialog.h"

#include <QApplication>
#include <QButtonGroup>
#include <QCheckBox>
#include <QClipboard>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QProcess>
#include <QProgressBar>
#include <QPushButton>
#include <QStandardItemModel>
#include <QTreeView>
#include <QVBoxLayout>

#include <KActionCollection>
#include <KComboBox>
#include <KIcon>
#include <KIconLoader>
#include <KLineEdit>
#include <KLocale>
#include <KMessageBox>
#include <KMimeType>
#include <KSeparator>
#include <KShortcut>
#include <KStandardAction>
#include <KUrlRequester>

// icon, name, regular expression, and default of the filter buttons
static const filterDefinition filters [] = {
    {QString("view-list-icons"), i18nc("filter: show all file types", "All"), KGetSortFilterProxyModel::NoFilter, true},
    {QString("video-x-generic"), i18n("Videos"), KGetSortFilterProxyModel::VideoFiles, false},
    {QString("audio-x-generic"), i18n("Audio"), KGetSortFilterProxyModel::AudioFiles, false},
    {QString("package-x-generic"), i18n("Archives"), KGetSortFilterProxyModel::CompressedFiles, false}
};

KGetLinkView::KGetLinkView(QWidget *parent)
  : KDialog(parent)
{
    setCaption(i18n("Import Links"));

    // proxy model to filter links
    m_proxyModel = new KGetSortFilterProxyModel();
    m_proxyModel->setDynamicSortFilter(true);
    m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    m_treeWidget = new QTreeView(this);
    m_treeWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_treeWidget->setModel(m_proxyModel);
    m_treeWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_treeWidget->setSelectionMode(QAbstractItemView::MultiSelection);
    m_treeWidget->setAlternatingRowColors(true);
    m_treeWidget->setRootIsDecorated(false);
    m_treeWidget->setSortingEnabled(true);
    m_treeWidget->setAllColumnsShowFocus(true);

    connect(m_treeWidget, SIGNAL(doubleClicked(const QModelIndex &)),
                        this, SLOT(uncheckItem(const QModelIndex &)));

    m_searchLine = new KLineEdit(this);
    m_searchLine->setClearButtonShown(true);
    m_searchLine->setClickMessage(i18n("Filter files here..."));
    connect(m_searchLine, SIGNAL(textChanged(QString)), SLOT(setTextFilter(QString)));

    // filter mode combobox [contains, does not contain]
    m_filterModeBox = new KComboBox(this);
    m_filterModeBox->addItem(i18n("Contains"), QVariant(KGetSortFilterProxyModel::Contain));
    m_filterModeBox->addItem(i18n("Does not Contain"), QVariant(KGetSortFilterProxyModel::DoesNotContain));
    connect(m_filterModeBox, SIGNAL(currentIndexChanged(int)), m_proxyModel, SLOT(setFilterMode(int)));
    connect(filterButtonsGroup, SIGNAL(buttonClicked(int)), SLOT(updateSelectionButtons()));

    setButtons(KDialog::None);

    filterButtonsGroup = new QButtonGroup(this);
    filterButtonsGroup->setExclusive(true);
    connect(filterButtonsGroup, SIGNAL(buttonClicked(int)), m_proxyModel, SLOT(setFilterType(int)));

    // filter buttons and filter line text box
    QHBoxLayout *filterLayout = new QHBoxLayout;
    filterLayout->addWidget(new QLabel(i18n("Show:")));

    for (uint i = 0; i < sizeof(filters) / sizeof(*filters); ++i)
    {
        filterLayout->addWidget(createFilterButton(filters[i].icon, filters[i].name,
                                filterButtonsGroup, filters[i].type, filters[i].defaultFilter));
    }

    filterLayout->addWidget(m_searchLine);
    filterLayout->addWidget(m_filterModeBox);

    // import url layout
    QLabel *importLabel = new QLabel(this);
    importLabel->setText(i18n("File with links to import:"));
    m_importButton = new QPushButton(i18n("Import links"), this);
    m_importButton->setIcon(KIcon("document-import"));
    m_importButton->setEnabled(false);
    m_urlRequester = new KUrlRequester(this);
    connect(m_urlRequester, SIGNAL(textChanged(const QString &)), SLOT(updateImportButtonStatus(const QString &)));

    m_importerLayout = new QHBoxLayout;
    m_importerLayout->addWidget(importLabel);
    m_importerLayout->addWidget(m_urlRequester);
    m_importerLayout->addWidget(m_importButton);

    m_linkImporter = 0;

    // import progressbar
    m_progressBar = new QProgressBar(this);
    m_progressBar->hide();

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setMargin(0);
    mainLayout->addLayout(m_importerLayout);
    mainLayout->addWidget(new KSeparator(this));
    mainLayout->addLayout(filterLayout);
    mainLayout->addWidget(m_treeWidget);
    mainLayout->addWidget(m_progressBar);

    // Bottoms buttons
    QHBoxLayout *bottomButtonsLayout = new QHBoxLayout;

    checkAllButton = new QPushButton(i18n("Select all"));
    uncheckAllButton = new QPushButton(i18n("Deselect all"));
    uncheckAllButton->setEnabled(false);
    m_checkSelectedButton = new QPushButton(i18n("Check selected"));
    m_checkSelectedButton->setEnabled(false);
    m_invertSelectionButton = new QPushButton(i18n("Invert selection"));
    m_invertSelectionButton->setEnabled(false);
    QCheckBox *showWebContentButton = new QCheckBox(i18n("Show web content"));
    downloadCheckedButton = new QPushButton( KIcon("kget"), i18n("Download checked"));
    downloadCheckedButton->setEnabled(false);
    QPushButton *cancelButton = new QPushButton(KIcon("dialog-cancel"), i18n("Cancel"));

    connect(cancelButton, SIGNAL(clicked()), this, SLOT(hide()));
    connect(checkAllButton, SIGNAL(clicked()), this, SLOT(checkAll()));
    connect(uncheckAllButton, SIGNAL(clicked()), this, SLOT(uncheckAll()));
    connect(m_checkSelectedButton, SIGNAL(clicked()), this, SLOT(slotCheckSelected()));
    connect(m_invertSelectionButton, SIGNAL(clicked()), this, SLOT(slotInvertSelection()));
    connect(downloadCheckedButton, SIGNAL(clicked()), this, SLOT(slotStartLeech()));
    connect(showWebContentButton, SIGNAL(stateChanged(int)), m_proxyModel, SLOT(setShowWebContent(int)));
    connect(m_importButton, SIGNAL(clicked()), this, SLOT(slotStartImport()));
    connect(m_treeWidget->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
                                            SLOT(selectionChanged()));

    bottomButtonsLayout->addWidget(m_checkSelectedButton);
    bottomButtonsLayout->addWidget(checkAllButton);
    bottomButtonsLayout->addWidget(uncheckAllButton);
    bottomButtonsLayout->addWidget(m_invertSelectionButton);
    bottomButtonsLayout->addWidget(showWebContentButton);

    mainLayout->addLayout(bottomButtonsLayout);

    mainLayout->addWidget(new KSeparator(this));

    QHBoxLayout *actionsLayout = new QHBoxLayout();

    actionsLayout->addStretch(10);
    actionsLayout->addWidget(downloadCheckedButton);
    actionsLayout->addWidget(cancelButton);

    mainLayout->addLayout(actionsLayout);

    QWidget *mainWidget = new QWidget(this);
    mainWidget->setLayout(mainLayout);
    setMainWidget(mainWidget);

    resize(600, 300);

    checkClipboard();
}

KGetLinkView::~KGetLinkView()
{
    delete(m_linkImporter);
}

void KGetLinkView::checkClipboard()
{
    QString clipboardContent = QApplication::clipboard()->text(QClipboard::Clipboard);

    if (clipboardContent.length() > 0)
    {
        delete m_linkImporter;

        m_linkImporter = new LinkImporter(this);

        connect(m_linkImporter, SIGNAL(finished()), SLOT(slotImportFinished()));

        m_linkImporter->checkClipboard(clipboardContent);
    }
}

void KGetLinkView::setLinks(const QList <QString> &links)
{
    m_links = QList <QString> ();
    foreach(const QString &link, links)
    {
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

    foreach (const QString &linkitem, links)
    {
        KUrl url(linkitem);
        QString file = url.fileName();
        if (file.isEmpty())
        {
            file = QString(url.host());
        }

        KMimeType::Ptr mt = KMimeType::findByUrl(linkitem, 0, true, true);

        QList<QStandardItem*> items;

        QStandardItem *item = new QStandardItem(file);
        item->setIcon(KIcon(mt->iconName()));
        item->setCheckable(true);
        item->setData(QVariant(url.fileName()), Qt::DisplayRole);
        item->setData(QVariant(mt->name()), Qt::UserRole); // used for filtering DownloadFilterType

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
    m_treeWidget->setColumnWidth(0, 200); // make the filename column bigger by default

    selectionChanged(); // adapt buttons to the new situation
}

void KGetLinkView::slotStartLeech()
{
    QStandardItemModel *model = qobject_cast<QStandardItemModel *>(m_proxyModel->sourceModel());
    if (model)
    {
        KUrl::List urls;

        for (int row=0 ; row < model->rowCount(); row++)
        {
            QStandardItem *checkeableItem = model->item(row, 1);

            if (checkeableItem->checkState() == Qt::Checked)
            {
                urls.append(KUrl(model->data(model->index(row, 4)).toString()));
            }
        }

        NewTransferDialog::instance()->showDialog(urls);
        accept(); // close the dialog
    }
    else
    {
        reject();
    }
}

void KGetLinkView::setPageUrl( const QString& url )
{
    setPlainCaption( i18n( "Links in: %1 - KGet", url ) );
}

void KGetLinkView::importUrl(const QString &url)
{
    if (url.isEmpty())
    {
        KUrl clipboardUrl = KUrl(QApplication::clipboard()->text(QClipboard::Clipboard).trimmed());
        if (clipboardUrl.isValid() &&
            ((!clipboardUrl.scheme().isEmpty() && !clipboardUrl.host().isEmpty()) ||
            (clipboardUrl.isLocalFile())))
        {
            m_urlRequester->setUrl(clipboardUrl);
        }
    }
    else
    {
        m_urlRequester->setUrl(KUrl(url));
        slotStartImport();
    }
}

void KGetLinkView::selectionChanged()
{
    QStandardItemModel *model = qobject_cast<QStandardItemModel *>(m_proxyModel->sourceModel());
    if (model)
    {
        const int modelRowCount = model->rowCount();
        bool buttonEnabled = false;
        int count = 0;

        for (int row = 0; row < modelRowCount; row++)
        {
            QStandardItem *checkeableItem = model->item(row, 1);

            if ((checkeableItem->checkState() == Qt::Checked))
            {
                buttonEnabled = true;

                // only count the checked files that are currently visible
                if (m_proxyModel->mapFromSource(model->index(row, 1)).isValid())
                {
                    count++;
                }
            }
        }

        checkAllButton->setEnabled( !(!modelRowCount || count == m_proxyModel->rowCount() ) );
        uncheckAllButton->setEnabled( count > 0 );
        m_invertSelectionButton->setEnabled( count > 0 );
        m_checkSelectedButton->setEnabled(m_treeWidget->selectionModel()->selectedIndexes().size() > 0);

        downloadCheckedButton->setEnabled(buttonEnabled);
    }
}

void KGetLinkView::setTextFilter(const QString &text)
{
    // TODO: escape user input for avoding malicious user input! (FiNEX)
    QString temp = text.isEmpty() ? m_searchLine->text() : text;
    m_proxyModel->setFilterWildcard(temp);

    updateSelectionButtons();
}

void KGetLinkView::updateSelectionButtons()
{
    const bool isFiltered = !m_searchLine->text().isEmpty() || (filterButtonsGroup->checkedId() != KGetSortFilterProxyModel::NoFilter);
    checkAllButton->setText(isFiltered ? i18n("Select all filtered") : i18n("Select all"));
    uncheckAllButton->setText(isFiltered ? i18n("Deselect all filtered") : i18n("Deselect all"));

    selectionChanged();
}

void KGetLinkView::checkAll()
{
    QStandardItemModel *itemsModel  = qobject_cast<QStandardItemModel *>(m_proxyModel->sourceModel());
    if (itemsModel)
    {
        for (int row = 0; row < m_proxyModel->rowCount(); row++)
        {
            const QModelIndex index = m_proxyModel->mapToSource(m_proxyModel->index(row, 3));
            QStandardItem *item = itemsModel->item(index.row(), 1);
            item->setCheckState(Qt::Checked);
        }
    }
}

void KGetLinkView::uncheckAll()
{
    QStandardItemModel *itemsModel  = qobject_cast<QStandardItemModel *>(m_proxyModel->sourceModel());
    if (itemsModel)
    {
        for (int row = 0; row < m_proxyModel->rowCount(); row++)
        {
            const QModelIndex index = m_proxyModel->mapToSource(m_proxyModel->index(row, 3));
            QStandardItem *item = itemsModel->item(index.row(), 1);
            item->setCheckState(Qt::Unchecked);
        }
    }
}

void KGetLinkView::uncheckItem(const QModelIndex &index)
{
    QStandardItemModel *model = qobject_cast<QStandardItemModel *>(m_proxyModel->sourceModel());
    if (model)
    {
        if (index.column() != 0)
        {
            QStandardItem *item = model->itemFromIndex(model->index(m_proxyModel->mapToSource(index).row(), 1));
            item->setCheckState(item->checkState() == Qt::Checked ? Qt::Unchecked : Qt::Checked);
        }
    }
}

void KGetLinkView::slotCheckSelected()
{
    QStandardItemModel *model = qobject_cast<QStandardItemModel *>(m_proxyModel->sourceModel());
    if (model)
    {
        foreach(const QModelIndex &index, m_treeWidget->selectionModel()->selectedIndexes())
        {
            QModelIndex sourceIndex = m_proxyModel->mapToSource(index);
            QStandardItem *item = model->item(sourceIndex.row(), 1);

            item->setCheckState(Qt::Checked);
        }
    }
}

void KGetLinkView::slotInvertSelection()
{
    QStandardItemModel *itemsModel  = qobject_cast<QStandardItemModel *>(m_proxyModel->sourceModel());
    if (itemsModel)
    {
        for (int row = 0; row < m_proxyModel->rowCount(); row++)
        {
            const QModelIndex index = m_proxyModel->mapToSource(m_proxyModel->index(row, 3));
            QStandardItem *item = itemsModel->item(index.row(), 1);
            item->setCheckState((item->checkState() == Qt::Checked) ? Qt::Unchecked : Qt::Checked);
        }
    }
}

void KGetLinkView::slotStartImport()
{
    delete m_linkImporter;

    m_linkImporter = new LinkImporter(m_urlRequester->url(), this);

    connect(m_linkImporter, SIGNAL(progress(int)), SLOT(slotImportProgress(int)));
    connect(m_linkImporter, SIGNAL(finished()), SLOT(slotImportFinished()));

    if (!m_urlRequester->url().isLocalFile())
    {
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

void KGetLinkView::updateImportButtonStatus(const QString &text)
{
    bool enabled = false;
    if (!text.isEmpty())
    {
        KUrl url(text);
        if (url.isValid())
        {
            enabled = true;
        }
    }
    m_importButton->setEnabled(enabled);
}

#include "kget_linkview.moc"
