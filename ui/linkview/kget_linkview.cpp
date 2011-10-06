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
#include "settings.h"
#include "kget_sortfilterproxymodel.h"
#include "core/kget.h"
#include "core/linkimporter.h"
#include "ui/newtransferdialog.h"

#include <QApplication>
#include <QClipboard>
#include <QMenu>
#include <QStandardItemModel>

#include <KAction>
#include <KActionCollection>
#include <KIcon>
#include <KIconLoader>
#include <KLocale>
#include <KMessageBox>
#include <KMimeType>
#include <KShortcut>
#include <KStandardAction>
#include <KWindowSystem>

KGetLinkView::KGetLinkView(QWidget *parent)
  : KGetSaveSizeDialog("KGetLinkView", parent),
    m_linkImporter(0),
    m_nameAction(0),
    m_urlAction(0)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setCaption(i18n("Import Links"));
    setButtons(0);
    
    if (parent) {
        KWindowInfo info = KWindowSystem::windowInfo(parent->winId(), NET::WMDesktop, NET::WMDesktop);
        KWindowSystem::setCurrentDesktop(info.desktop());
        KWindowSystem::forceActiveWindow(parent->winId());
    }

    // proxy model to filter links
    m_proxyModel = new KGetSortFilterProxyModel(1, this);
    m_proxyModel->setDynamicSortFilter(true);
    m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    QWidget *widget = new QWidget(this);
    ui.setupUi(widget);
    
    m_proxyModel->setShowWebContent(ui.showWebContent->isChecked());

    ui.filterMode->addItem(i18n("Contains"), KGetSortFilterProxyModel::Contain);
    ui.filterMode->addItem(i18n("Does Not Contain"), KGetSortFilterProxyModel::DoesNotContain);

    // set the Icons
    ui.importLinks->setIcon(KIcon("document-import"));
    ui.showCombo->addItem(KIcon("view-list-icons"), i18n("All"), KGetSortFilterProxyModel::NoFilter);
    ui.showCombo->addItem(KIcon("video-x-generic"), i18n("Videos"), KGetSortFilterProxyModel::VideoFiles);
    ui.showCombo->addItem(KIcon("image-x-generic"), i18n("Images"), KGetSortFilterProxyModel::ImageFiles);
    ui.showCombo->addItem(KIcon("audio-x-generic"), i18n("Audio"), KGetSortFilterProxyModel::AudioFiles);
    ui.showCombo->addItem(KIcon("package-x-generic"), i18n("Archives"), KGetSortFilterProxyModel::CompressedFiles );

    ui.treeView->setModel(m_proxyModel);
    ui.progressBar->hide();

    //creates pattern syntax menu for the text filter
    m_patternSyntaxMenu = new QMenu(i18nc("of a filter, e.g. RegExp or Wildcard", "Pattern Syntax"), this);
    QAction *wildcardAction = new QAction(i18n("Escape Sequences"), this);
    wildcardAction->setCheckable(true);
    wildcardAction->setChecked(Settings::linkViewFilterPatternSyntax() == Wildcard);
    QAction *regExpAction = new QAction(i18n("Regular Expression"), this);
    regExpAction->setCheckable(true);
    regExpAction->setChecked(Settings::linkViewFilterPatternSyntax() == RegExp);
    QActionGroup *actionGroup = new QActionGroup(this);
    actionGroup->addAction(wildcardAction);
    actionGroup->addAction(regExpAction);
    m_patternSyntaxMenu->addActions(actionGroup->actions());

    //Filter for name/url actions
    QActionGroup *columnGroup = new QActionGroup(this);
    m_nameAction = new KAction(i18nc("name of a file", "Name"), this);
    m_nameAction->setCheckable(true);
    m_nameAction->setChecked(true);
    m_urlAction = new KAction(i18n("URL"), this);
    m_urlAction->setCheckable(true);
    columnGroup->addAction(m_nameAction);
    columnGroup->addAction(m_urlAction);
    connect(columnGroup, SIGNAL(triggered(QAction*)), this, SLOT(slotFilterColumn(QAction*)));

    connect(wildcardAction, SIGNAL(toggled(bool)), this, SLOT(wildcardPatternToggled(bool)));
    connect(ui.treeView, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(uncheckItem(QModelIndex)));
    connect(ui.textFilter, SIGNAL(textChanged(QString)), SLOT(setTextFilter(QString)));
    connect(ui.textFilter, SIGNAL(aboutToShowContextMenu(QMenu*)), this, SLOT(contextMenuDisplayed(QMenu*)));
    connect(ui.filterMode, SIGNAL(currentIndexChanged(int)), this, SLOT(slotFilterModeChanged(int)));
    connect(ui.showCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(slotMimeTypeChanged(int)));
    connect(ui.showCombo, SIGNAL(currentIndexChanged(int)), SLOT(updateSelectionButtons()));
    connect(ui.urlRequester, SIGNAL(textChanged(QString)), SLOT(updateImportButtonStatus(QString)));
    connect(ui.urlRequester, SIGNAL(urlSelected(KUrl)), SLOT(slotStartImport()));
    connect(ui.selectAll, SIGNAL(clicked()), this, SLOT(checkAll()));
    connect(ui.deselectAll, SIGNAL(clicked()), this, SLOT(uncheckAll()));
    connect(ui.invertSelection, SIGNAL(clicked()), this, SLOT(slotInvertSelection()));
    connect(this, SIGNAL(accepted()), this, SLOT(slotStartLeech()));
    connect(ui.showWebContent, SIGNAL(stateChanged(int)), m_proxyModel, SLOT(setShowWebContent(int)));
    connect(ui.importLinks, SIGNAL(clicked()), this, SLOT(slotStartImport()));
    connect(ui.treeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            SLOT(selectionChanged()));
    connect(ui.dialogButtonBox, SIGNAL(rejected()), SLOT(reject()));

    setMainWidget(widget);
    QPushButton *download = ui.dialogButtonBox->addButton(i18nc("Download the items which have been selected","&Download"),
                                                          QDialogButtonBox::AcceptRole,
                                                          this,
                                                          SLOT(accept()));
    download->setIcon(KIcon("kget"));

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

void KGetLinkView::setLinks(const QStringList &links)
{
    m_links = links;
    showLinks(m_links, false);
}

void KGetLinkView::showLinks(const QStringList &links, bool urlRequestVisible)
{
    ui.importWidget->setVisible(urlRequestVisible);

    QStandardItemModel *model = new QStandardItemModel(0, 5, this);

    model->setHeaderData(0, Qt::Horizontal, i18n("Auxiliary header"));
    model->setHeaderData(1, Qt::Horizontal, i18n("File Name"));
    model->setHeaderData(2, Qt::Horizontal, i18n("Description"));
    model->setHeaderData(3, Qt::Horizontal, i18nc("list header: type of file", "File Type"));
    model->setHeaderData(4, Qt::Horizontal, i18n("Location (URL)"));

    foreach (const QString &linkitem, links)
    {
        KUrl url;
        KMimeType::Ptr mt;

        if (linkitem.contains(QLatin1String("url "), Qt::CaseInsensitive) &&
            linkitem.contains(QLatin1String("type "), Qt::CaseInsensitive)) {
            const QStringList items = linkitem.split(QLatin1Char(' '), QString::SkipEmptyParts);
            const int count = items.count();
            int index = items.indexOf(QLatin1String("url"));
            if (index > -1 && index+1 < count)
                url = items.at(index+1);
            index = items.indexOf(QLatin1String("type"));
            if (index > -1 && index+1 < count)
                mt = KMimeType::mimeType(items.at(index+1));
        } else {
            url = linkitem;
            mt = KMimeType::findByUrl(linkitem, 0, true, true);
        }

        kDebug(5001) << "Adding:" << linkitem;
        
        QString file = url.fileName();
        if (file.isEmpty())
            file = QString(url.host());

        QString mimeTypeName, mimeTypeIcon, mimeTypeComment;
        if (mt) {
            mimeTypeName = mt->name();
            mimeTypeIcon = mt->iconName();
            mimeTypeComment = mt->comment();
        }

        QStandardItem *item = new QStandardItem(file);
        item->setIcon(KIcon(mimeTypeIcon));
        item->setCheckable(true);
        item->setCheckState(Qt::Checked);
        item->setData(QVariant(url.fileName()), Qt::DisplayRole);
        item->setData(QVariant(mimeTypeName), Qt::UserRole); // used for filtering DownloadFilterType

        QList<QStandardItem*> items;
        QStandardItem *number = new QStandardItem();
        number->setData(model->rowCount(), Qt::DisplayRole);//used for inital sorting
        items << number;
        items << item;
        items << new QStandardItem();
        items << new QStandardItem(mimeTypeComment);
        items << new QStandardItem(url.prettyUrl());

        model->insertRow(model->rowCount(), items);
    }

    connect(model, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(selectionChanged()));
    m_proxyModel->setSourceModel(model);
    m_proxyModel->setFilterKeyColumn(1);
    m_proxyModel->sort(0);

    ui.treeView->header()->hideSection(0);
    ui.treeView->setColumnWidth(1, 200); // make the filename column bigger by default

    selectionChanged(); // adapt buttons to the new situation
}

void KGetLinkView::slotMimeTypeChanged(int index)
{
    m_proxyModel->setFilterType(ui.showCombo->itemData(index).toInt());
}

void KGetLinkView::slotFilterModeChanged(int index)
{
    m_proxyModel->setFilterMode(ui.filterMode->itemData(index).toInt());
}

void KGetLinkView::slotFilterColumn(QAction *action)
{
    //FIXME make this not depend on "magic numbers"?
    m_proxyModel->setFilterColumn(action == m_urlAction ? 4 : 1);
}

void KGetLinkView::slotStartLeech()
{
    QStandardItemModel *model = qobject_cast<QStandardItemModel *>(m_proxyModel->sourceModel());
    if (model)
    {
        KUrl::List urls;

        for (int row = 0; row < model->rowCount(); row++)
        {
            QStandardItem *checkeableItem = model->item(row, 1);

            if (checkeableItem->checkState() == Qt::Checked)
            {
                urls.append(KUrl(model->data(model->index(row, 4)).toString()));
            }
        }

        NewTransferDialogHandler::showNewTransferDialog(urls);
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
            ui.urlRequester->setUrl(clipboardUrl);
        }
    }
    else
    {
        ui.urlRequester->setUrl(KUrl(url));
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

        ui.selectAll->setEnabled( !(!modelRowCount || count == m_proxyModel->rowCount() ) );
        ui.deselectAll->setEnabled( count > 0 );
        ui.invertSelection->setEnabled( count > 0 );

        enableButtonOk(buttonEnabled);
    }
}

void KGetLinkView::setTextFilter(const QString &text)
{
    // TODO: escape user input for avoding malicious user input! (FiNEX)
    QString temp = text.isEmpty() ? ui.textFilter->text() : text;
    if (Settings::linkViewFilterPatternSyntax() == Wildcard) {
        m_proxyModel->setFilterWildcard(temp);
    } else {
        QRegExp rx(temp, Qt::CaseSensitive, QRegExp::RegExp2);
        m_proxyModel->setFilterRegExp(rx);
    }

    updateSelectionButtons();
}

void KGetLinkView::updateSelectionButtons()
{
    const bool isFiltered = !ui.textFilter->text().isEmpty() || (ui.showCombo->currentIndex() != KGetSortFilterProxyModel::NoFilter);
    ui.selectAll->setText(isFiltered ? i18n("&Select All Filtered") : i18n("&Select All"));
    ui.deselectAll->setText(isFiltered ? i18n("D&eselect All Filtered") : i18n("D&eselect All"));

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
        foreach(const QModelIndex &index, ui.treeView->selectionModel()->selectedIndexes())
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

    m_linkImporter = new LinkImporter(ui.urlRequester->url(), this);

    connect(m_linkImporter, SIGNAL(progress(int)), SLOT(slotImportProgress(int)));
    connect(m_linkImporter, SIGNAL(finished()), SLOT(slotImportFinished()));

    if (!ui.urlRequester->url().isLocalFile())
    {
        m_linkImporter->copyRemoteFile();
    }

    m_linkImporter->start();
    ui.progressBar->show();
}

void KGetLinkView::slotImportProgress(int progress)
{
    ui.progressBar->setValue(progress);
}

void KGetLinkView::slotImportFinished()
{
    ui.progressBar->hide();
    m_links = m_linkImporter->links();
    showLinks(m_links, true);
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
    ui.importLinks->setEnabled(enabled);
}

void KGetLinkView::contextMenuDisplayed(QMenu *menu)
{
    menu->addSeparator();
    menu->addMenu(m_patternSyntaxMenu);
    menu->addSeparator()->setText(i18n("Filter Column"));
    menu->addAction(m_nameAction);
    menu->addAction(m_urlAction);
}


void KGetLinkView::wildcardPatternToggled(bool enabled)
{
    if (enabled) {
        Settings::setLinkViewFilterPatternSyntax(Wildcard);
    } else {
        Settings::setLinkViewFilterPatternSyntax(RegExp);
    }
}

#include "kget_linkview.moc"
