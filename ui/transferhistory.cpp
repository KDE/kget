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

#include <QFile>
#include <QDomElement>
#include <QDomNodeList>
#include <QMenu>
#include <QFileSystemWatcher>
#include <QFontMetrics>
#include <QDateTime>
#include <QStandardItem>
#include <QFontMetrics>

#include <KDebug>
#include <KStandardDirs>
#include <KMessageBox>
#include <KToolBar>
#include <KTreeWidgetSearchLine>
#include <KRun>
#include <KGlobalSettings>
#include <kio/global.h>

TransferHistory::TransferHistory(QWidget *parent)
    : KDialog(parent),
    m_rangeType(0)
{
    setCaption(i18n("Transfer History"));
    setButtons(KDialog::Close);
    //Setup Ui-Parts from Designer
    QWidget *mainWidget = new QWidget(this);

    Ui::TransferHistory widget;
    widget.setupUi(mainWidget);

    m_treeWidget = new RangeTreeWidget(this);
    slotLoadRangeType(TransferHistory::Date);

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

    m_verticalLayout->addWidget(m_treeWidget);

    watcher = new QFileSystemWatcher();
    watcher->addPath(KStandardDirs::locateLocal("appdata", QString()));
    kDebug(5001) << watcher->directories();

    connect(m_actionDelete_Selected, SIGNAL(triggered()), this, SLOT(slotDeleteTransfer()));
    connect(m_actionDownload, SIGNAL(triggered()), this, SLOT(slotDownload()));
    connect(m_openFile, SIGNAL(triggered()), this, SLOT(slotOpenFile()));
    connect(m_clearButton, SIGNAL(clicked()), this, SLOT(slotClear()));
    connect(watcher, SIGNAL(directoryChanged(const QString &)), this, SLOT(slotLoadRangeType()));
    connect(m_rangeTypeCombobox, SIGNAL(activated(int)), this, SLOT(slotLoadRangeType(int)));
}

void TransferHistory::slotDeleteTransfer()
{
    QString transferName = m_treeWidget->currentItem(0)->text();
    QString filename = KStandardDirs::locateLocal("appdata", "transferhistory.kgt");
    QString error;
    int line;
    int column;

    m_treeWidget->removeRow(m_treeWidget->currentIndex().row(), m_treeWidget->currentIndex().parent());

    QDomDocument doc("tempHistory");
    QFile file(filename);
    if (!doc.setContent(&file, &error, &line, &column)) 
    {
        kDebug(5001) << "Error1" << error << line << column;
        file.close();
        return;
    }
    file.close();

    QDomElement root = doc.documentElement();

    kDebug(5001) << "Load file" << filename;

    QDomNodeList list = root.elementsByTagName("Transfer");

    int nItems = list.length();

    QDomElement elementToRemove;
    for (int i = 0 ; i < nItems ; i++)
    {
        QDomElement element = list.item(i).toElement();
        kDebug(5001) << element.attribute("Source", "") << "----" << transferName;
        if (element.attribute("Source", "") == transferName) {
            elementToRemove = element;
        }
    }

    doc.documentElement().removeChild(elementToRemove);
    file.remove();

    if (!file.open(QIODevice::ReadWrite)) {
        KMessageBox::error(0, i18n("History-File cannot be opened correctly!"), i18n("Error"), 0);
    }
    else {
        QTextStream stream(&file);
        doc.save(stream, 0);
        file.close();
    }

    // reload the treewidget
    slotLoadRangeType(m_rangeType);
}

void TransferHistory::slotAddTransfers()
{
    QString filename = KStandardDirs::locateLocal("appdata", "transferhistory.kgt");
    QString error;
    int line;
    int column;

    QDomDocument doc("tempHistory");
    QFile file(filename);
    if (!doc.setContent(&file, &error, &line, &column)) 
    {
        kDebug(5001) << "Error1" << error << line << column;
        file.close();
        return;
    }

    QDomElement root = doc.documentElement();

    kDebug(5001) << "Load file" << filename;

    QDomNodeList list = root.elementsByTagName("Transfer");

    int nItems = list.length();

    for (int i = 0 ; i < nItems ; i++)
    {
        QDomElement dom = list.item(i).toElement();
        defaultItems.append(dom);
        QVariantList attributeList;
        attributeList.append(dom.attribute("Source"));
        attributeList.append(dom.attribute("Dest"));
        attributeList.append(QDateTime::fromString(dom.attribute("Time")).date().toString());
        attributeList.append(KIO::convertSize(dom.attribute("Size").toInt()));
        attributeList.append(dom.attribute("State"));

        int data = 0;
        if(m_rangeType == TransferHistory::Date) {
            QDate date = QDateTime::fromString(dom.attribute("Time")).date();
            data = date.daysTo(QDate::currentDate());
        }
        else {
            data = dom.attribute("Size").toInt();
            kDebug() << "Setting data to " << data;
        }
        m_treeWidget->add(data, attributeList);
    }
    doc.clear();
    file.close();
}

void TransferHistory::slotClear()
{
    QString filename = KStandardDirs::locateLocal("appdata", "transferhistory.kgt");
    QFile file(filename);
    file.remove();
    slotLoadRangeType();
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

void TransferHistory::slotDownload()
{
    NewTransferDialog::showNewTransferDialog(m_treeWidget->currentItem(0)->text());
}

void TransferHistory::contextMenuEvent(QContextMenuEvent *event)
{
    Q_UNUSED(event)

    if(m_treeWidget->currentIndex().parent().isValid()) {
        QMenu *contextMenu = new QMenu(this);
        contextMenu->addAction(m_actionDownload);
        contextMenu->addAction(m_actionDelete_Selected);

        if (m_treeWidget->currentItem(4)->text() == "Finished")
            contextMenu->addAction(m_openFile);
        contextMenu->exec(QCursor::pos());
    }
}

void TransferHistory::slotOpenFile()
{
    new KRun(m_treeWidget->currentItem(1)->text(), this, true, false);
}

void TransferHistory::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event);
    disconnect(watcher, SIGNAL(directoryChanged(const QString &)), this, SLOT(slotAddTransfers()));//Prevent reloading of TransferHistory when saving
    deleteLater();
}

void TransferHistory::slotLoadRangeType(int type)
{
    if(type >= 0) {
        m_rangeType = type;
    }

    QFontMetrics *font = new QFontMetrics(KGlobalSettings::generalFont());
    m_treeWidget->clear();

    m_treeWidget->setLabels(QStringList() << "Source-File" << "Destination" << "Time" << "File Size" << "Status");

    switch(m_rangeType)
    {
        case TransferHistory::Size :
            m_treeWidget->addRange(0, 1024 * 1024, i18n("Less than 1MiB"));
            m_treeWidget->addRange(1024 * 1024, 1024 * 1024 * 10, i18n("Between 1MiB-10MiB"));
            m_treeWidget->addRange(1024 * 1024 * 10, 1024 * 1024 * 100, i18n("Between 10MiB-100MiB"));
            m_treeWidget->addRange(1024 * 1024 * 100, 1024 * 1024 *1024, i18n("Between 100MiB-1GiB"));
            m_treeWidget->addRange(1024 * 1024 * 1024, 1024 * 1024 * 1024 * 10, i18n("More than 1GiB"));
            break;
        default:
            m_treeWidget->addRange(0, 1, i18n("Today"));
            m_treeWidget->addRange(1, 7, i18n("Last week"));
            m_treeWidget->addRange(7, 30, i18n("Last month"));
            m_treeWidget->addRange(30, -1, i18n("A long time ago"));
    }

    QList<int> list = Settings::historyColumnWidths();

    if (!list.isEmpty())
    {
        int j = 0;
        foreach (int i, list)
        {
            m_treeWidget->setColumnWidth(j, i);
            j++;
        }
    }
    else
    {
        kDebug(5001);
        m_treeWidget->setColumnWidth(0, 200);
        m_treeWidget->setColumnWidth(1, 250);
        m_treeWidget->setColumnWidth(2, font->width(QDate::currentDate().toString()));
        m_treeWidget->setColumnWidth(3, font->width("1500000 KiB"));
        m_treeWidget->setColumnWidth(4, font->width(i18nc("the transfer has been finished", "Finished")));
    }

    slotAddTransfers();
}
