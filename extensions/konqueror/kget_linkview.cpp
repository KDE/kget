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
#include <ktoolbar.h>
#include <KTreeWidgetSearchLine>

#include <QAction>
#include <QPixmap>
#include <QProcess>
#include <QTreeWidget>
#include <QtDBus>
#include <QVBoxLayout>

KGetLinkView::KGetLinkView(QWidget *parent)
  : KDialog(parent)
{
    setPlainCaption(i18n("KGet"));

    QStringList headers;
    headers << i18n("File Name") << i18n("Description")
            << i18n("File Type") << i18n("Location (URL)");

    m_treeWidget = new QTreeWidget(this);
    m_treeWidget->setHeaderLabels(headers);
    m_treeWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_treeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_treeWidget->setAlternatingRowColors(true);
    m_treeWidget->setRootIsDecorated(false);
    m_treeWidget->setSortingEnabled(true);
    m_treeWidget->setAllColumnsShowFocus(true);
    m_treeWidget->setColumnWidth(0, 200); // make the filename column bigger by default

    connect(m_treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(slotStartLeech()));
    connect(m_treeWidget, SIGNAL(itemSelectionChanged()), SLOT(selectionChanged()));

    KTreeWidgetSearchLine *searchLine = new KTreeWidgetSearchLine(this, m_treeWidget);
    searchLine->setClickMessage(i18n("Filter files here..."));
    connect(searchLine, SIGNAL(textChanged(QString)), SLOT(updateSelectAllText(QString)));

    setButtons(KDialog::Cancel | KDialog::User1 | KDialog::User2);

    setButtonText(KDialog::User1, i18n("Download selected"));
    setButtonIcon(KDialog::User1, KIcon("kget"));
    enableButton(KDialog::User1, false);
    connect(this, SIGNAL(user1Clicked()), SLOT(slotStartLeech()));

    setButtonText(KDialog::User2, i18n("Select all"));
    connect(this, SIGNAL(user2Clicked()), m_treeWidget, SLOT(selectAll()));

    setDefaultButton(KDialog::User1);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setMargin(0);
    mainLayout->addWidget(searchLine);
    mainLayout->addWidget(m_treeWidget);

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
    m_treeWidget->clear();

    QList<QTreeWidgetItem *> items;
    foreach (LinkItem* linkitem, links) {
        QString file = linkitem->url.fileName();
        if ( file.isEmpty() )
            file = linkitem->url.host();

        QStringList itemStringList;
        itemStringList << file << linkitem->text
                       << linkitem->mimeType << linkitem->url.prettyUrl();
        QTreeWidgetItem *treeWidgetItem = new QTreeWidgetItem(m_treeWidget, itemStringList);
        treeWidgetItem->setIcon(0, KIcon(linkitem->icon));
        items.append(treeWidgetItem);
    }
    m_treeWidget->insertTopLevelItems(0, items);

    m_treeWidget->sortItems(0, Qt::AscendingOrder);
}

void KGetLinkView::slotStartLeech()
{
    QStringList urls;

    QTreeWidgetItemIterator it(m_treeWidget);
    while (*it) {
        if ((*it)->isSelected()) {
            QString url = (*it)->text(3);
            urls.append(url);
        }
        ++it;
    }

    if(!QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.kget"))
    {
        QProcess *kgetProcess = new QProcess(this);
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
    enableButton(KDialog::User1, !m_treeWidget->selectedItems().isEmpty());
}

void KGetLinkView::updateSelectAllText(const QString &text)
{
    setButtonText(KDialog::User2, text.isEmpty() ? i18n("Select all") : i18n("Select all filtered"));
}

#include "kget_linkview.moc"
