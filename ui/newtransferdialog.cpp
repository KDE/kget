/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2007 by Javier Goday <jgoday@gmail.com>
   Copyright (C) 2008 by Lukas Appelhans <l.appelhans@gmx.de>

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
#include <QDir>

#include <KLocale>
#include <KListWidget>
#include <QListWidgetItem>
#include <KLineEdit>
#include <KComboBox>
#include <KDebug>
#include <KFileDialog>

NewTransferDialog::NewTransferDialog(QWidget *parent)
    : KDialog(parent),
      m_multiple(false),
      listWidget(0),
      urlRequester(0)
{
    setCaption(i18n("New Download"));
    showButtonSeparator(true);

    QWidget *mainWidget = new QWidget(this);

    Ui::NewTransferWidget widget;
    widget.setupUi(mainWidget);

    m_gridLayout1 = widget.gridLayout1;
    m_titleWidget = widget.titleWidget;
    m_destRequester = widget.destRequester;
    m_groupComboBox = widget.groupComboBox;
    m_defaultFolderButton = widget.defaultFolderButton;
    m_groupLabel = widget.groupLabel;
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
    KUrl::List urls = list;
    NewTransferDialog *dialog = new NewTransferDialog();
    dialog->setMultiple(true);

    if (urls.count() == 2)
    {
        if (urls.at(1).protocol().isEmpty())//When we have no protocol in the second filename, then it's the filename (thrown by konqueror)
        {
            dialog->setDestinationFileName(urls.takeAt(1).pathOrUrl());//We set it to the correct filename
        }
    }
    dialog->setSource(urls);

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
        if (urlRequester)
            urlRequester->hide();
        listWidget = new KListWidget();
        m_gridLayout1->addWidget(listWidget, 0, 1, 1, 1);
        m_destRequester->setMode(KFile::Directory);
    }
    else
    {
        if (listWidget)
            listWidget->hide();
        urlRequester = new KLineEdit();
        urlRequester->setClearButtonShown(true);
        m_gridLayout1->addWidget(urlRequester, 0, 1, 1, 1);
        m_destRequester->setMode(KFile::File);
    }
}

void NewTransferDialog::setSource(const QString &srcUrl)
{
    KUrl m_srcUrl = srcUrl;
    urlRequester->clear();
    if (m_srcUrl.isEmpty())
        m_srcUrl = KUrl(QApplication::clipboard()->text(QClipboard::Clipboard).trimmed());

    if (m_srcUrl.isValid() && !m_srcUrl.protocol().isEmpty())
        urlRequester->insert(m_srcUrl.prettyUrl());

    connect(urlRequester, SIGNAL(textChanged(const QString &)), SLOT(setDefaultDestination()));
}

void NewTransferDialog::setSource(const KUrl::List &list)
{
    if (list.count() == 1)
    {
        setMultiple(false);
        setSource(list.first().url());
    }

    KUrl::List::const_iterator it = list.begin();
    KUrl::List::const_iterator itEnd = list.end();

    for (; it!=itEnd ; ++it)
    {
        if (it->url() != KUrl(it->url()).fileName())
        {
            kDebug(5001) << "Insert " + it->url();
            QListWidgetItem *newItem = new QListWidgetItem(it->pathOrUrl(), listWidget);
            newItem->setCheckState(Qt::Checked);
        }
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

void NewTransferDialog::setDestination(const QStringList &list)
{
    QStringList m_list = list;
    kDebug(5001) << m_list;
    QString filename = KUrl(destination()).fileName();
    if (filename.isEmpty())
        filename = source().first().fileName();

    for (int i=0;i < list.count();i++)
    {
        m_list[i].append('/' + filename);
    }
    kDebug(5001) << m_list;

    m_destRequester->comboBox()->insertItems(0, m_list);
}

void NewTransferDialog::setDefaultDestination()
{
    setDestination(KGet::defaultFolders(source().first().path(), transferGroup()));
}

void NewTransferDialog::setDestinationFileName(const QString &filename)
{
    m_destRequester->setUrl(m_destRequester->url().path(KUrl::AddTrailingSlash) + filename);
}

QString NewTransferDialog::destination() const
{
    return m_destRequester->url().prettyUrl();
}

QString NewTransferDialog::transferGroup() const
{
    return m_groupComboBox->currentText();
}

void NewTransferDialog::showNewTransferDialog(NewTransferDialog *dialog)
{
    QString destDir;
    
    dialog->setDefaultDestination();

    //if (dialog->destination().isEmpty())
    //    dialog->setDestination(QDir::home().path() + '/' + dialog->source().first().fileName());

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
            QString dir;
            if (dialog->multiple())
                dir = destDir; 
            else
                dir = KUrl(destDir).directory();

            if(dialog->m_defaultFolderButton->checkState() == Qt::Checked)
            {
                Settings::setDefaultDirectory(dir);
                Settings::setUseDefaultDirectory(true);
                Settings::self()->writeConfig();
            }
            else
            {
                Settings::setLastDirectory(dir);
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
    // properties of the m_destRequester combobox
    m_destRequester->comboBox()->setDuplicatesEnabled(false);
    m_destRequester->comboBox()->setUrlDropsEnabled(true);
    m_destRequester->comboBox()->setEditable(true);
    m_destRequester->fileDialog()->setKeepLocation(true);

    // transfer groups
    m_groupComboBox->addItems(KGet::transferGroupNames());

    if (KGet::transferGroupNames().count() < 2)
    {
        m_groupComboBox->hide();
        m_groupLabel->hide();
    }

    m_groupComboBox->setCurrentIndex(0);
    m_titleWidget->setPixmap(KIcon("document-new").pixmap(22, 22), KTitleWidget::ImageLeft);

    connect(m_groupComboBox, SIGNAL(currentIndexChanged(int)), SLOT(setDefaultDestination()));
}

#include "newtransferdialog.moc"
