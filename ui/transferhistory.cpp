/* This file is part of the KDE project

   Copyright (C) 2007 by Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "transferhistory.h"
#include "ui/newtransferdialog.h"

#include <QFile>
#include <QDomElement>
#include <QDomNodeList>
#include <QMenu>
#include <QFileSystemWatcher>
#include <QFontMetrics>
#include <QDateTime>

#include <KDebug>
#include <KStandardDirs>
#include <KMessageBox>
#include <KToolBar>
#include <KTreeWidgetSearchLine>
#include <KRun>
#include <KGlobalSettings>

TransferHistory::TransferHistory(QWidget *parent)
    : KDialog(parent)
{
    setCaption(i18n("Transfer History"));
    setButtons(KDialog::Close);
    //Setup Ui-Parts from Designer
    QWidget *mainWidget = new QWidget(this);

    Ui::TransferHistory widget;
    widget.setupUi(mainWidget);

    m_gridLayout = widget.gridLayout;
    m_treeWidget = widget.treeWidget;
    m_treeWidget->setRootIsDecorated(false);

    QFontMetrics *font = new QFontMetrics(KGlobalSettings::generalFont());
    kDebug(5001) << font->width(QDateTime::currentDateTime().toString()) << font->width(" 150 MiB ") << font->width("Finished");

    m_treeWidget->setColumnWidth(0, 265);
    m_treeWidget->setColumnWidth(1, 200);
    m_treeWidget->setColumnWidth(2, font->width(QDateTime::currentDateTime().toString()));
    m_treeWidget->setColumnWidth(3, font->width("150 MiB"));
    m_treeWidget->setColumnWidth(4, font->width("Finished"));
    m_hboxLayout = widget.hboxLayout;
    m_searchBar = widget.searchBar;
    m_searchBar->setTreeWidget(m_treeWidget);
    m_clearButton = widget.clearButton;
    m_clearButton->setIcon(KIcon("edit-clear-history"));
    m_actionDelete_Selected = widget.actionDelete_Selected;
    m_actionDelete_Selected->setIcon(KIcon("edit-delete"));
    m_actionDownload = widget.actionDownload;
    m_actionDownload->setIcon(KIcon("document-new"));
    m_openFile = new QAction(KIcon("document-open"), "&Open File", this);
    setMainWidget(mainWidget);
    setInitialSize(QSize(800, 400));

    slotAddTransfers();

    watcher = new QFileSystemWatcher();
    watcher->addPath(KStandardDirs::locateLocal("appdata", QString()));
    kDebug(5001) << watcher->directories();

    connect(m_actionDelete_Selected, SIGNAL(triggered()), this, SLOT(slotDeleteTransfer()));
    connect(m_actionDownload, SIGNAL(triggered()), this, SLOT(slotDownload()));
    connect(m_openFile, SIGNAL(triggered()), this, SLOT(slotOpenFile()));
    connect(m_clearButton, SIGNAL(clicked()), this, SLOT(slotClear()));
    connect(watcher, SIGNAL(directoryChanged(const QString &)), this, SLOT(slotAddTransfers()));
    connect(this, SIGNAL(destroyed()), this, SLOT(slotSave()));
    //connect(this, SIGNAL(rejected()), this, SLOT(slotWriteDefault()));
}

void TransferHistory::slotDeleteTransfer()
{
    int index = m_treeWidget->indexOfTopLevelItem(m_treeWidget->currentItem());
    delete m_treeWidget->takeTopLevelItem(index);
}

void TransferHistory::slotAddTransfers()
{
    QString filename = KStandardDirs::locateLocal("appdata", "transferhistory.kgt");
    QString error;
    int line;
    int column;

    QDomDocument doc("tempHistory");
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
        return;
    if (!doc.setContent(&file, &error, &line, &column)) 
    {
        kDebug(5001) << "Error1" << error << line << column;
        file.close();
        return;
    }
    if(file.reset())
        kDebug(5001) << "resettingn the file, to have a clean starting point to readd";
    file.close();

    QDomElement root = doc.documentElement();

    kDebug(5001) << "Load file" << filename;

    QDomNodeList list = root.elementsByTagName("Transfer");

    int nItems = list.length();

    for (int i = 0 ; i < nItems ; i++)
    {
        QDomElement dom = list.item(i).toElement();
        defaultItems.append(dom);
        QStringList attributeList;
        attributeList.append(dom.attribute("Source"));
        attributeList.append(dom.attribute("Dest"));
        attributeList.append(dom.attribute("Time"));
        attributeList.append(dom.attribute("Size"));
        attributeList.append(dom.attribute("State"));
        QTreeWidgetItem *item = new QTreeWidgetItem(attributeList);
        m_treeWidget->addTopLevelItem(item);
        kDebug(5001) << attributeList;

        foreach (QDomElement element, defaultItems)
        {
            kDebug(5001) << element.attribute("Source");
        }
    }
    doc.clear();
    file.remove();
}

void TransferHistory::slotClear()
{
     m_treeWidget->clear();
}

void TransferHistory::slotWriteDefault()
{
    kDebug(5001);
    if (!save)
    {
        QString filename = KStandardDirs::locateLocal("appdata", "transferhistory.kgt");
        QFile file(filename);
        file.remove();
        QDomDocument *doc;
        QDomElement root;

        if (!file.exists())
        {
            doc = new QDomDocument("Transfers");
            root = doc->createElement("Transfers");
            doc->appendChild(root);
        }
        else
        {
            doc = new QDomDocument();
            doc->setContent(&file);
            file.close();
            root = doc->documentElement();
            doc->appendChild(root);
        }

        foreach (QDomElement e, defaultItems)
        {
            kDebug(5001) << e.attribute("Source");
            root.appendChild(e);
        }

        if (!file.open(QIODevice::ReadWrite))
            KMessageBox::error(0, i18n("History-File cannot be opened correctly!"), i18n("Error"), 0);

        QTextStream stream(&file);
        doc->save(stream, 0);
        file.close();
    }
}

void TransferHistory::slotSave()
{
    kDebug(5001);
    QString filename = KStandardDirs::locateLocal("appdata", "transferhistory.kgt");
    QFile file(filename);
    file.remove();
    QDomDocument *doc;
    QDomElement root;

    if (!file.exists())
    {
        doc = new QDomDocument("Transfers");
        root = doc->createElement("Transfers");
        doc->appendChild(root);
    }
    else
    {
        doc = new QDomDocument();
        doc->setContent(&file);
        file.close();
        root = doc->documentElement();
        doc->appendChild(root);
    }

    foreach (QTreeWidgetItem *item, m_treeWidget->findItems("", Qt::MatchContains | Qt::MatchRecursive))
    {
        QDomElement e = doc->createElement("Transfer");
        root.appendChild(e);

        e.setAttribute("Source", item->text(0));
        e.setAttribute("Dest", item->text(1));
        e.setAttribute("Time", item->text(2));
        e.setAttribute("Size", item->text(3));
        e.setAttribute("State", item->text(4));

        kDebug(5001) << e.attribute("Source");
    }

    if (!file.open(QIODevice::ReadWrite))
        KMessageBox::error(0, i18n("History-File cannot be opened correctly!"), i18n("Error"), 0);

    QTextStream stream(&file);
    doc->save(stream, 0);
    file.close();
    save = true;

    close();
}

void TransferHistory::slotDownload()
{
    if (m_treeWidget->indexOfTopLevelItem(m_treeWidget->currentItem()) == -1)
        return;
    slotSave();
    NewTransferDialog::showNewTransferDialog(m_treeWidget->currentItem()->text(0));
}

void TransferHistory::contextMenuEvent(QContextMenuEvent *event)
{
    Q_UNUSED(event);
    if (m_treeWidget->indexOfTopLevelItem(m_treeWidget->currentItem()) == -1)
        return;
    QMenu *contextMenu = new QMenu(this);
    contextMenu->addAction(m_actionDownload);
    contextMenu->addAction(m_actionDelete_Selected);
    if (m_treeWidget->currentItem()->text(4) == "Finished")
        contextMenu->addAction(m_openFile);
    contextMenu->exec(QCursor::pos());
}

void TransferHistory::slotOpenFile()
{
    new KRun(m_treeWidget->currentItem()->text(1), this, true, false);
}

void TransferHistory::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event);
    disconnect(watcher, SIGNAL(directoryChanged(const QString &)), this, SLOT(slotAddTransfers()));//Prevent reloading of TransferHistory when saving
    slotSave();
    deleteLater();
}
