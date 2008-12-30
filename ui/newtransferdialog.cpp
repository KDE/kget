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
#include <KWindowSystem>

class NewTransferDialog::Private : public QObject
{
public:
    Private(QObject *parent=0) : QObject(parent),
            m_multiple(0),
            m_displayed(0)
    {
        listWidget = 0;
        urlRequester = 0;
    }

    KUrl::List sources() const
    {
        KUrl::List list;
        if (m_multiple)
        {
            for (int i = 0; i != listWidget->count(); i++) {
                if (listWidget->item(i)->checkState() == Qt::Checked)
                    list << KUrl(listWidget->item(i)->text());
            }
        }
        else
            list << KUrl(urlRequester->text());
        return list;
    }

    QString destination() const
    {
        return m_destRequester->url().prettyUrl();
    }

    QString transferGroup() const
    {
        return m_groupComboBox->currentText();
    }

    /**
    * Determines where is a multiple (listwidget) or single (kurlrequester) transfer
    */
    void setMultiple(bool value)
    {
        m_multiple = value;

        if (value)
        {
            if (urlRequester) {
                urlRequester->hide();
                urlRequester->deleteLater();
                urlRequester = 0;
            }
            if (!listWidget) {
                listWidget = new KListWidget();
                m_gridLayout1->addWidget(listWidget, 0, 1, 1, 1);
                m_destRequester->setMode(KFile::Directory);
            }
        }
        else
        {
            if (listWidget) {
                listWidget->hide();
                listWidget->deleteLater();
                listWidget = 0;
            }
            if (!urlRequester) {
                urlRequester = new KLineEdit();
                urlRequester->setClearButtonShown(true);
                m_gridLayout1->addWidget(urlRequester, 0, 1, 1, 1);
                m_destRequester->setMode(KFile::File);
            }
        }
    }

    void setDestinationFileName(const QString &filename)
    {
        m_destRequester->setUrl(m_destRequester->url().path(KUrl::AddTrailingSlash) + filename);
    }

    void setSource(const KUrl::List &list)
    {
        if (list.count() <= 1)
        {
            KUrl m_srcUrl = list.first().url();
            urlRequester->clear();
            if (m_srcUrl.isEmpty())
                m_srcUrl = KUrl(QApplication::clipboard()->text(QClipboard::Clipboard).trimmed());

            if (m_srcUrl.isValid() && !m_srcUrl.protocol().isEmpty())
                urlRequester->insert(m_srcUrl.prettyUrl());
        }
        else {
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
    }

    void setDestination(const KUrl::List &sources, const QStringList &list)
    {
        Q_UNUSED(sources)

        m_destRequester->comboBox()->clear();
        m_destRequester->clear();

        QStringList m_list = list;
        kDebug(5001) << m_list;
        QString filename = destination();

        kDebug() << "Seting destination :multiple=" << m_multiple << " and filename=" << filename;
        if (!filename.isEmpty() && m_multiple) {
            filename = KUrl(filename).directory();
        }
        else if (urlRequester) {
            filename = KUrl(urlRequester->text()).fileName();
        }

        for (int i=0;i < list.count();i++)
        {
            if (!m_list.at(i).endsWith('/'))
                m_list[i].append('/');
            m_list[i].append(filename);
        }
        kDebug(5001) << m_list;

        m_destRequester->comboBox()->insertItems(0, m_list);
    }

    void prepareGui()
    {
        // properties of the m_destRequester combobox
        m_destRequester->comboBox()->setDuplicatesEnabled(false);
        m_destRequester->comboBox()->setUrlDropsEnabled(true);
        m_destRequester->comboBox()->setEditable(true);
        m_destRequester->fileDialog()->setKeepLocation(true);

        // transfer groups
        m_groupComboBox->addItems(KGet::transferGroupNames());

        m_groupComboBox->setCurrentIndex(0);
        m_titleWidget->setPixmap(KIcon("document-new").pixmap(22, 22), KTitleWidget::ImageLeft);
    }

    void clear()
    {
        if(urlRequester) {
            urlRequester->clear();
        }
        if(listWidget) {
            listWidget->clear();
        }
        m_destRequester->comboBox()->clear();
        m_destRequester->clear();
        m_displayed = false;
    }

    KListWidget *listWidget;
    KLineEdit *urlRequester;

    QGridLayout *m_gridLayout1;
    KTitleWidget *m_titleWidget;

    KUrlComboRequester *m_destRequester;
    KComboBox *m_groupComboBox;
    QCheckBox *m_defaultFolderButton;
    QLabel *m_groupLabel;

    bool m_multiple;
    bool m_displayed; // determines whenever the dialog is already displayed or not (to add new sources)
};

K_GLOBAL_STATIC(NewTransferDialog, globalInstance)

NewTransferDialog::NewTransferDialog(QWidget *parent) : KDialog(parent),
    m_window(0), m_sources()
{
    d = new NewTransferDialog::Private(this);

    setModal(true);
    setCaption(i18n("New Download"));
    showButtonSeparator(true);

    QWidget *mainWidget = new QWidget(this);

    Ui::NewTransferWidget widget;
    widget.setupUi(mainWidget);

    d->m_gridLayout1 = widget.gridLayout1;
    d->m_titleWidget = widget.titleWidget;
    d->m_destRequester = widget.destRequester;
    d->m_groupComboBox = widget.groupComboBox;
    d->m_defaultFolderButton = widget.defaultFolderButton;
    d->m_groupLabel = widget.groupLabel;

    setMainWidget(mainWidget);
    
    d->prepareGui();
    resizeDialog();

    connect(d->m_groupComboBox, SIGNAL(currentIndexChanged(int)), SLOT(setDefaultDestination()));

    qAddPostRoutine(globalInstance.destroy);
}

NewTransferDialog::~NewTransferDialog()
{
    delete d;
    qRemovePostRoutine(globalInstance.destroy);
}

NewTransferDialog *NewTransferDialog::instance(QWidget *parent)
{
    globalInstance->m_window = parent;

    return globalInstance;
}

void NewTransferDialog::showDialog(const QString &srcUrl)
{
    KUrl::List list;
    list << KUrl(srcUrl);

    showDialog(list);
}

void NewTransferDialog::showDialog(const KUrl::List &list)
{
    m_sources << list;

    kDebug() << "SET SOURCES " << list << " MULTIPLE " << (m_sources.size () > 1);
    d->setMultiple(m_sources.size() > 1);

    if (list.count() == 2)
    {
        if (list.at(1).protocol().isEmpty())//When we have no protocol in the second filename, then it's the filename (thrown by konqueror)
        {
            d->setDestinationFileName(list.at(1).pathOrUrl());//We set it to the correct filename
        }
    }

    d->setSource(m_sources);

    resizeDialog();
    prepareDialog();
}

void NewTransferDialog::setDefaultDestination()
{
    d->setDestination(m_sources, KGet::defaultFolders(m_sources.first().path(), d->transferGroup()));
}

void NewTransferDialog::prepareDialog()
{
    setDefaultDestination();

    if(m_window) {
        KWindowInfo info = KWindowSystem::windowInfo(m_window->winId(), NET::WMDesktop, NET::WMDesktop);
        KWindowSystem::setCurrentDesktop(info.desktop());
        KWindowSystem::forceActiveWindow(m_window->winId());
    }

    if (!d->m_displayed) {
        d->m_displayed = true;

        KDialog::exec();

        if (result() == KDialog::Accepted)
        {
            QString destDir = d->destination();
            m_sources = d->sources();

            destDir = KUrl(destDir).path();

            QString dir;
            if (QFileInfo(destDir).isDir())
                dir = destDir;
            else
                dir = KUrl(destDir).directory();

            if(d->m_defaultFolderButton->checkState() == Qt::Checked)
            {
                Settings::setDefaultDirectory(dir);
                Settings::setUseDefaultDirectory(true);
                Settings::self()->writeConfig();
            }
            else {
                Settings::setLastDirectory(dir);
                Settings::self()->writeConfig();
            }
            
            kDebug(5001) << m_sources;
            if (m_sources.count() == 1)
                KGet::addTransfer(m_sources.takeFirst(), destDir, d->transferGroup());
            else
                KGet::addTransfer(m_sources, destDir, d->transferGroup());
        }

        m_sources.clear();
        d->clear();
    }
}

void NewTransferDialog::resizeDialog()
{
    if (KGet::transferGroupNames().count() < 2)
    {
        d->m_groupComboBox->hide();
        d->m_groupLabel->hide();
    }
}

#include "newtransferdialog.moc"
