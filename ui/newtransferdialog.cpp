/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2007 by Javier Goday <jgoday@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "newtransferdialog.h"

#include "core/kget.h"
#include "core/transfertreemodel.h"
#include "settings.h"

#include <QWidget>
#include <QApplication>
#include <QClipboard>

#include <KLocale>
#include <KListWidget>
#include <QListWidgetItem>
#include <KLineEdit>
#include <KComboBox>
#include <KDebug>

NewTransferDialog::NewTransferDialog(QWidget *parent)
    : KDialog(parent),
      m_multiple(false)
{
    setCaption(i18n("New Download"));
    setMinimumSize(400, 300);
    showButtonSeparator(true);

    QWidget *mainWidget = new QWidget(this);

    Ui::NewTransferWidget widget;
    widget.setupUi(mainWidget);

    m_gridLayout1 = widget.gridLayout1;
    m_titleWidget = widget.titleWidget;
    m_folderRequester = widget.folderRequester;
    m_groupComboBox = widget.groupComboBox;
    m_defaultFolderButton = widget.defaultFolderButton;
    setMainWidget(mainWidget);
    prepareGui();
}

void NewTransferDialog::showNewTransferDialog(const QString &srcUrl)
{
    NewTransferDialog *dialog = new NewTransferDialog();
    dialog->setMultiple(false);
    dialog->setSource(srcUrl);

    NewTransferDialog::showNewTransferDialog(dialog);
}

void NewTransferDialog::showNewTransferDialog(const KUrl::List &list)
{
    NewTransferDialog *dialog = new NewTransferDialog();
    dialog->setMultiple(true);
    dialog->setSource(list);

    NewTransferDialog::showNewTransferDialog(dialog);
}

bool NewTransferDialog::multiple() const 
{
    return m_multiple;
}

void NewTransferDialog::setMultiple(bool value)
{
    m_multiple = value;
    if (multiple())
    {
        listWidget = new KListWidget();
        m_gridLayout1->addWidget(listWidget, 0, 1, 1, 1);
    }
    else
    {
        urlRequester = new KLineEdit();
        urlRequester->setClearButtonShown(true);
        m_gridLayout1->addWidget(urlRequester, 0, 1, 1, 1);
    }
}

void NewTransferDialog::setSource(const QString &srcUrl)
{
    urlRequester->clear();
    if (srcUrl.isEmpty()) 
    {
        KUrl clipboardUrl = KUrl(QApplication::clipboard()->text(QClipboard::Clipboard).trimmed());
        if (clipboardUrl.isValid())
            urlRequester->insert(clipboardUrl.url());
    }
    else 
    {
        urlRequester->insert(srcUrl);
    }
}

void NewTransferDialog::setSource(const KUrl::List &list)
{
    KUrl::List::const_iterator it = list.begin();
    KUrl::List::const_iterator itEnd = list.end();

    for (; it!=itEnd ; ++it)
    {
        QListWidgetItem *newItem = new QListWidgetItem(it->url(), listWidget);
        newItem->setCheckState(Qt::Checked);
    }
}

KUrl::List NewTransferDialog::source() const
{
    KUrl::List list;
    if (multiple())
    {
        foreach (QListWidgetItem *item, listWidget->findItems(QString('*'), Qt::MatchWrap | Qt::MatchWildcard))
        {
            if (item->checkState() == Qt::Checked)
                list.append(KUrl(item->text()));
        }
    }
    else
    {
        list.append(KUrl(urlRequester->text()));
    }
    return list;
}

void NewTransferDialog::setDestination(const QString &destination)
{
    m_folderRequester->comboBox()->insertItem(0, destination);
    m_folderRequester->comboBox()->setCurrentIndex(0);
}

QString NewTransferDialog::destination() const
{
    return m_folderRequester->url().url();
}

QString NewTransferDialog::transferGroup() const
{
    return m_groupComboBox->currentText();
}

void NewTransferDialog::showNewTransferDialog(NewTransferDialog *dialog)
{
    QString destDir;

    if (Settings::useDefaultDirectory())
#ifdef Q_OS_WIN //krazy:exclude=cpp
        destDir = Settings::defaultDirectory().remove("file:///");
#else
        destDir = Settings::defaultDirectory().remove("file://");
#endif

    if (!dialog->source().isEmpty())
    {
        QString checkExceptions = KGet::getSaveDirectoryFromExceptions(dialog->source().takeFirst());

        if (Settings::enableExceptions() && !checkExceptions.isEmpty())
            destDir = checkExceptions;
    }

    if (!Settings::lastDirectory().isEmpty() &&
        QString::compare(Settings::lastDirectory(), Settings::defaultDirectory()) != 0)
    {
        dialog->setDestination(Settings::lastDirectory());
    }
    
    if (!destDir.isEmpty())
        dialog->setDestination(destDir);

    dialog->exec();

    KUrl::List srcUrls = dialog->source();
    destDir = dialog->destination();

    if (dialog->result() == KDialog::Accepted)
    {
#ifdef Q_OS_WIN //krazy:exclude=cpp
            destDir = destDir.remove("file:///");
#else
            destDir = destDir.remove("file://");
#endif
            if(dialog->m_defaultFolderButton->checkState() == Qt::Checked)
            {
                Settings::setDefaultDirectory(destDir);
                Settings::setUseDefaultDirectory(true);
                Settings::self()->writeConfig();
            }
            else
            {
                Settings::setLastDirectory(destDir);
                Settings::self()->writeConfig();
            }
            kDebug(5001) << srcUrls;
            if (srcUrls.count() == 1)
                KGet::addTransfer(srcUrls.takeFirst(), destDir, dialog->transferGroup());
            else
                KGet::addTransfer(srcUrls, destDir, dialog->transferGroup());
    }
}

void NewTransferDialog::prepareGui()
{
    // Only select directories
    m_folderRequester->setMode(KFile::Directory);

    // properties of the m_folderRequester combobox
    m_folderRequester->comboBox()->setDuplicatesEnabled(false);

    // transfer groups
    m_groupComboBox->addItems(KGet::transferGroupNames());

    m_groupComboBox->setCurrentIndex(0);
    m_titleWidget->setPixmap(KIcon("document-new").pixmap(22, 22), KTitleWidget::ImageLeft);
}

#include "newtransferdialog.moc"
