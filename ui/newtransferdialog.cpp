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
#include <QVBoxLayout>
#include <QApplication>
#include <QClipboard>

#include <KUrl>
#include <KLocale>
#include <KComboBox>
#include <KTitleWidget>

NewTransferDialog::NewTransferDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption(i18n("New Download"));

    m_transferWidget = new NewTransferWidget();

    QVBoxLayout *layout = new QVBoxLayout();
    QWidget *mainWidget = new QWidget();
    KTitleWidget *title = new KTitleWidget(this);

    title->setText(i18n("New Download"));
    title->setPixmap(KIcon("document-new").pixmap(22), KTitleWidget::ImageLeft);
    layout->addWidget(title);
    layout->addWidget(m_transferWidget);
    mainWidget->setLayout(layout);

    setMainWidget(mainWidget);
}

NewTransferDialog::~NewTransferDialog()
{
}

NewTransferWidget *NewTransferDialog::transferWidget()
{
    return m_transferWidget;
}

void NewTransferDialog::slotButtonClicked(int button)
{
    if(button == KDialog::Cancel) {
        transferWidget()->setUrl(QString());
        transferWidget()->setFolderPath(QString());
    }

    KDialog::slotButtonClicked(button);
}

void NewTransferDialog::showNewTransferDialog()
{
    QString srcUrl;
    QString destDir;

    KUrl clipboardUrl = KUrl(QApplication::clipboard()->text(QClipboard::Clipboard).trimmed());
    if (clipboardUrl.isValid())
        srcUrl = clipboardUrl.url();

    if (Settings::useDefaultDirectory())
#ifdef Q_OS_WIN //krazy:exclude=cpp
        destDir = Settings::defaultDirectory().remove("file:///");
#else
        destDir = Settings::defaultDirectory().remove("file://");
#endif

    QString checkExceptions = KGet::getSaveDirectoryFromExceptions(srcUrl);
    if (Settings::enableExceptions() && !checkExceptions.isEmpty())
        destDir = checkExceptions;

    NewTransferDialog dialog;
    dialog.transferWidget()->setFolderPath(destDir);
    dialog.transferWidget()->setUrl(srcUrl);

    dialog.exec();

    srcUrl = dialog.transferWidget()->url();
    destDir = dialog.transferWidget()->folderPath();

    if(!srcUrl.isEmpty() && !destDir.isEmpty() && KGet::isValidSource(srcUrl)) {
#ifdef Q_OS_WIN //krazy:exclude=cpp
        destDir = destDir.remove("file:///");
#endif
        destDir = destDir.remove("file://");

        if(dialog.transferWidget()->setAsDefaultFolder()) {
            Settings::setDefaultDirectory(destDir);
            Settings::self()->writeConfig();
        }
        KGet::addTransfer(srcUrl, destDir, dialog.transferWidget()->groupName());
    }
}

NewTransferWidget::NewTransferWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);

    // Only select directories
    folderRequester->setMode(KFile::Directory);

    // properties of the folderRequester combobox
    folderRequester->comboBox()->setDuplicatesEnabled(false);

    // transfer groups
    groupComboBox->addItems(KGet::transferGroupNames());

    // common usefull folders for the combobox of the url requester
    if(!Settings::defaultDirectory().isEmpty() && !Settings::defaultDirectory().endsWith(QDir::homePath())) {
#ifdef Q_OS_WIN //krazy:exclude=cpp
        folderRequester->comboBox()->addItem(Settings::defaultDirectory().remove("file:///"));
#endif
        folderRequester->comboBox()->addItem(Settings::defaultDirectory().remove("file://"));
    }
    if(!Settings::lastDirectory().isEmpty()) {
        folderRequester->comboBox()->addItem(Settings::lastDirectory());
    }

    folderRequester->comboBox()->addItem(QDir::homePath());

    groupComboBox->setCurrentIndex(0);
    folderRequester->comboBox()->setCurrentIndex(0);
}

void NewTransferWidget::setFolderPath(QString path)
{
    if(!path.isEmpty()) {
        folderRequester->setPath(path);
    }
}

void NewTransferWidget::setUrl(QString url)
{
    urlRequester->setText(url);
}

QString NewTransferWidget::folderPath() const
{
    return folderRequester->url().url();
}

QString NewTransferWidget::url() const
{
    return urlRequester->text();
}

QString NewTransferWidget::groupName() const
{
    return groupComboBox->currentText();
}

bool NewTransferWidget::setAsDefaultFolder() const
{
    return (defaultFolderButton->checkState() == Qt::Checked);
}
