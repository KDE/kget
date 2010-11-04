/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2007 by Javier Goday <jgoday@gmail.com>
   Copyright (C) 2008 - 2009 by Lukas Appelhans <l.appelhans@gmx.de>
   Copyright (C) 2010 by Matthias Fuchs <mat69@gmx.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "newtransferdialog.h"

#include "core/kget.h"
#include "core/transfertreemodel.h"
#include "core/transfergrouphandler.h"
#include "core/plugin/transferfactory.h"
#include "settings.h"

#include <QWidget>
#include <QApplication>
#include <QClipboard>
#include <QDir>
#include <QFileInfo>

#include <KLocale>
#include <KListWidget>
#include <QListWidgetItem>
#include <KLineEdit>
#include <KComboBox>
#include <KDebug>
#include <KFileDialog>
#include <KWindowSystem>

K_GLOBAL_STATIC(NewTransferDialogHandler, newTransferDialogHandler)


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
    
    bool isEmpty()
    {
        if (urlRequester) {
            return urlRequester->text().trimmed().isEmpty();
        } else if (listWidget) {
            return !listWidget->count();
        }
        return true;
    }

    KUrl::List sources() const
    {
        KUrl::List list;
        if (m_multiple)
        {
            for (int i = 0; i != listWidget->count(); i++) {
                if (listWidget->item(i)->checkState() == Qt::Checked)
                    list << KUrl(listWidget->item(i)->text().trimmed());
            }
        }
        else
            list << KUrl(urlRequester->text().trimmed());
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
                connect(urlRequester, SIGNAL(textChanged(QString)), parent(), SLOT(urlChanged(QString)));
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

        const QList<TransferGroupHandler*> groups = KGet::groupsFromExceptions(sources().first());
        if (!groups.isEmpty()) {
            m_groupComboBox->setCurrentIndex(m_groupComboBox->findText(groups.first()->name()));
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

        kDebug(5001) << "Seting destination :multiple=" << m_multiple << " and filename=" << filename;
        if (!filename.isEmpty() && m_multiple) {
            filename = KUrl(filename).directory();
        }
        else if (urlRequester) {
            filename = KUrl(urlRequester->text().trimmed()).fileName();
        }

        for (int i=0;i < list.count();i++)
        {
            if (!m_list.at(i).endsWith('/'))
                m_list[i].append('/');
            m_list[i].append(filename);
        }

        const QString downloadPath = KGet::generalDestDir();
        if (!downloadPath.isEmpty()) {
            m_list << downloadPath;
        }
        kDebug(5001) << m_list;

        m_destRequester->comboBox()->insertItems(0, m_list);
        QString group = m_groupComboBox->currentText();
        TransferGroupHandler * current = 0;
        foreach (TransferGroupHandler * handler, KGet::allTransferGroups()) {
            if (handler->name() == group) {
                current = handler;
            }
        }
        if (current && !current->defaultFolder().isEmpty()) {
            if (m_destRequester->comboBox()->findText(current->defaultFolder()) == -1)
                m_destRequester->comboBox()->addItem(current->defaultFolder());
            m_destRequester->comboBox()->setCurrentIndex(m_destRequester->comboBox()->findText(current->defaultFolder()));
        } else if (current) {
            m_destRequester->comboBox()->setCurrentIndex(m_destRequester->comboBox()->findText(downloadPath));
        }
    }

    void prepareGui()
    {
        // properties of the m_destRequester combobox
        m_destRequester->comboBox()->setDuplicatesEnabled(false);
        m_destRequester->comboBox()->setUrlDropsEnabled(true);
        m_destRequester->comboBox()->setEditable(true);
        m_destRequester->fileDialog()->setKeepLocation(true);

        m_titleWidget->setPixmap(KIcon("document-new").pixmap(22, 22), KTitleWidget::ImageLeft);
    }

    void clear()
    {
        if (urlRequester) {
            urlRequester->clear();
        }
        if (listWidget) {
            listWidget->clear();
        }
        m_destRequester->comboBox()->clear();
        m_destRequester->clear();
        m_displayed = false;
        m_groupComboBox->clear();
        //KGet::addTransferView(m_groupComboBox);
        m_groupComboBox->clear();
        foreach (TransferGroupHandler * group, KGet::allTransferGroups())
            m_groupComboBox->addItem(KIcon(group->iconName()), group->name());
        m_groupComboBox->setCurrentIndex(0);
    }

    KListWidget *listWidget;
    KLineEdit *urlRequester;

    QGridLayout *m_gridLayout1;
    KTitleWidget *m_titleWidget;

    KUrlComboRequester *m_destRequester;
    KComboBox *m_groupComboBox;
    QLabel *m_groupLabel;

    bool m_multiple;
    bool m_displayed; // determines whenever the dialog is already displayed or not (to add new sources)
};

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
    d->m_groupLabel = widget.groupLabel;

    setMainWidget(mainWidget);
    
    d->prepareGui();
    resizeDialog();
    d->clear();

    connect(d->m_groupComboBox, SIGNAL(currentIndexChanged(int)), SLOT(setDefaultDestination()));
}

NewTransferDialog::~NewTransferDialog()
{
}

void NewTransferDialog::showDialog(const KUrl::List &list, const QString &suggestedFileName)
{
    m_sources << list;

    d->clear();//Let's clear the old stuff
    kDebug(5001) << "SET SOURCES " << list << " MULTIPLE " << (m_sources.size () > 1);
    d->setMultiple(m_sources.size() > 1);

    if (!m_sources.isEmpty()) {
        if (list.count() == 1 && !suggestedFileName.isEmpty())
            d->setDestinationFileName(suggestedFileName);

        d->setSource(m_sources);
    }

    resizeDialog();
    prepareDialog();
}

void NewTransferDialog::setDefaultDestination()
{
    //if (m_sources.isEmpty()) {
    //    return;
    //}
    QStringList list;
    foreach (TransferGroupHandler *handler, KGet::allTransferGroups()) {
        if (!handler->defaultFolder().isEmpty())
            list << handler->defaultFolder();
    }
    d->setDestination(m_sources, list);
    if (!m_sources.isEmpty())
        urlChanged(m_sources.first().path());
    //d->setDestination(m_sources, KGet::defaultFolders(m_sources.first().path(), d->transferGroup()));
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
        kDebug(5001) << "Exec the dialog!";
        d->m_displayed = true;

        KDialog::exec();

        if (result() == KDialog::Accepted)
        {
            QString destDir = d->destination();
            m_sources = d->sources();

            destDir = KUrl(destDir).toLocalFile();

            QString dir;
            if (QFileInfo(destDir).isDir())
                dir = destDir;
            else
                dir = KUrl(destDir).directory();

            Settings::setLastDirectory(dir);
            Settings::self()->writeConfig();
            
            kDebug(5001) << m_sources;
            if (m_sources.count() == 1)
                KGet::addTransfer(m_sources.takeFirst(), destDir, QString(), d->transferGroup());
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

bool NewTransferDialog::isEmpty()
{
    return d->isEmpty();
}

void NewTransferDialog::urlChanged(const QString &text)
{
    if (d->m_multiple)
        return;
    KUrl url(text.trimmed());
    //if (d->m_destRequester->url()->isEmpty())
    //    d->setDestination(m_sources, QStringList());
    if (QFileInfo(d->m_destRequester->url().toLocalFile()).isDir())
        d->setDestinationFileName(url.fileName());

    kDebug() << url << url.fileName() << d->m_destRequester->url().fileName();
}


/**
 * NOTE some checks in this class might seem redundant, though target is to display as few dialogs, and then preferable
 * the NewTransferDialog, to the user as possible i.e. if not enough information -- e.g. no destination folder
 * determinable, ...-- is present for a url or a group of urls they won't be added as transfer,
 * instead the NewTransferDialog will be shown
 *
 * This also tries to add as many transfers as possible with one run, to ensure a high speed
 */
NewTransferDialogHandler::NewTransferDialogHandler(QObject *parent)
  : QObject(parent),
    m_nextJobId(0)
{
}

NewTransferDialogHandler::~NewTransferDialogHandler()
{
    if (m_dialog) {
        if (!m_dialog->isVisible()) {
            delete m_dialog;
        } else {
            m_dialog->deleteLater();
        }
    }
}


void NewTransferDialogHandler::showNewTransferDialog(const KUrl &url, QWidget *parent)
{
    showNewTransferDialog(url.isEmpty() ? KUrl::List() : KUrl::List() << url, parent);
}

void NewTransferDialogHandler::showNewTransferDialog(KUrl::List urls, QWidget *parent)
{
    if (urls.isEmpty()) {
        newTransferDialogHandler->createDialog(urls, QString(), parent);
        return;
    }

    QHash<int, UrlData>::iterator itUrls = newTransferDialogHandler->m_urls.insert(newTransferDialogHandler->m_nextJobId, UrlData());
    (*itUrls).parent = parent;
    QString folder;
    QString suggestedFileName;

    ///Only two urls defined, check if second one is a path or a file name
    if (urls.count() == 2) {
        const KUrl lastUrl = urls.last();

        //check if last url is a file path, either absolute or relative
        if (lastUrl.isLocalFile()) {
            if (QDir::isAbsolutePath(lastUrl.toLocalFile())) {
                //second url is a file path, use this one
                folder = lastUrl.directory(KUrl::AppendTrailingSlash);
                suggestedFileName = lastUrl.fileName();
                urls.removeLast();
            } else {
                //second url is just a file name
                suggestedFileName = lastUrl.fileName(KUrl::ObeyTrailingSlash);
                urls.removeLast();
            }
        } else if (!lastUrl.isValid() || (lastUrl.scheme().isEmpty() && lastUrl.directory().isEmpty())) {
            // Sometimes valid filenames are not recognised by KURL::isLocalFile(), they are marked as invalid then
            suggestedFileName = lastUrl.url();
            urls.removeLast();
        }
    }

    ///More than two urls defined, and last is local and will be used as destination directory
    if (urls.count() > 2 && urls.last().isLocalFile()) {
        if (!QFileInfo(urls.last().toLocalFile()).isDir()) {
            folder = urls.last().directory(KUrl::AppendTrailingSlash);
        } else {
            folder = urls.last().path(KUrl::AddTrailingSlash);
        }
        urls.removeLast();
    }

    //add a folder or suggestedFileName if they are valid
    if (!folder.isEmpty() && KGet::isValidDestDirectory(folder)) {
        (*itUrls).folder = folder;
    }
    if (!suggestedFileName.isEmpty()) {
        (*itUrls).suggestedFileName = suggestedFileName;
    }

    newTransferDialogHandler->m_numJobs[newTransferDialogHandler->m_nextJobId] = urls.count();
    foreach (const KUrl &url, urls) {
        //needed to avoid when protocols like the desktop protocol is used, see bko:185283
        KIO::StatJob *job = KIO::mostLocalUrl(url, KIO::HideProgressInfo);
        job->setProperty("jobId", (newTransferDialogHandler->m_nextJobId));
        connect(job, SIGNAL(result(KJob*)), newTransferDialogHandler, SLOT(slotMostLocalUrlResult(KJob*)));
    }

    ++(newTransferDialogHandler->m_nextJobId);
}

void NewTransferDialogHandler::slotMostLocalUrlResult(KJob *j)
{
    KIO::StatJob *job = static_cast<KIO::StatJob*>(j);
    const int jobId = job->property("jobId").toInt();

    if (job->error()) {
        kWarning(5001) << "An error happened for asdfasdf";
    } else {
        m_urls[jobId].urls << job->mostLocalUrl();
    }
    --m_numJobs[jobId];

    if (m_numJobs[jobId] <= 0) {
        handleUrls(jobId);
    }
}

void NewTransferDialogHandler::handleUrls(const int jobId)
{
    QHash<int, UrlData>::iterator itUrls = m_urls.find(jobId);
    if (itUrls == m_urls.end()) {
        kWarning(5001) << "JobId" << jobId << "was not defined, could not handle urls for it.";
        return;
    }

    KUrl::List urls = (*itUrls).urls;
    const QString folder = (*itUrls).folder;
    const QString suggestedFileName = (*itUrls).suggestedFileName;
    QWidget *parentWidget = (*itUrls).parent;

    KUrl::List::iterator it = urls.begin();
    while (it != urls.end()) {
        //TODO rework the way KGet::isValidSource is done, so that the user is not constantly asked in case of existing urls
        if (KGet::isValidSource(*it)) {
            ++it;
        } else {
            kDebug(5001) << "Not downloading source:" << (*it);
            it = urls.erase(it);
        }
    }

    QList<KGet::TransferData> data;

    ///Just one file to download, with a specified suggestedFileName, handle if possible
    if (!suggestedFileName.isEmpty() && (urls.count() == 1)) {
        const KUrl sourceUrl = urls.first();
        const QList<TransferGroupHandler*> groups = KGet::groupsFromExceptions(sourceUrl);
        const QString groupName = (groups.isEmpty() ? QString() : groups.first()->name());
        const QString defaultFolder = (groups.isEmpty() ? QString() : groups.first()->defaultFolder());

        if (!folder.isEmpty()) {
            const KUrl destUrl = KGet::getValidDestUrl(KUrl(folder), sourceUrl);
            if (destUrl.isEmpty()) {
                kWarning(5001) << "Could not create a valid dest url for" << sourceUrl;
            } else {
                data << KGet::TransferData(sourceUrl, destUrl, groupName);
            }
            urls.removeFirst();
        } else if (!Settings::directoriesAsSuggestion() && KGet::isValidDestDirectory(defaultFolder)) {
            const KUrl destUrl = KGet::getValidDestUrl(KUrl(defaultFolder), sourceUrl);
            if (destUrl.isEmpty()) {
                kWarning(5001) << "Could not create a valid dest url for" << sourceUrl;
            } else {
                data << KGet::TransferData(sourceUrl, destUrl, groupName);
            }
            urls.removeFirst();
        }
    }

    ///A valid folder has been defined, use that for downloading
    if (!folder.isEmpty()) {
        //find the associated groups first, we just need the first matching group though
        const QList<TransferGroupHandler*> groups = KGet::allTransferGroups();
        foreach (TransferGroupHandler *group, groups) {
            if (urls.isEmpty()) {
                break;
            }

            const QString groupName = group->name();
            const QStringList patterns = group->regExp().pattern().split(',');

            //find all urls where a group can be identified
            it = urls.begin();
            while (it != urls.end()) {
                const KUrl sourceUrl = *it;
                if (KGet::matchesExceptions(sourceUrl, patterns)) {
                    const KUrl destUrl = KGet::getValidDestUrl(KUrl(folder), sourceUrl);
                    if (destUrl.isEmpty()) {
                        kWarning(5001) << "Could not create a valid dest url for" << sourceUrl;
                        it = urls.erase(it);
                        continue;
                    }

                    data << KGet::TransferData(sourceUrl, destUrl, groupName);
                    it = urls.erase(it);
                } else {
                    ++it;
                }
            }
        }

        //there are still some unhandled urls, i.e. for those no group could be found, add them with an empty group
            foreach (const KUrl &sourceUrl, urls) {
                const KUrl destUrl = KGet::getValidDestUrl(KUrl(folder), sourceUrl);
                if (destUrl.isEmpty()) {
                    kWarning(5001) << "Could not create a valid dest url for" << sourceUrl;
                    continue;
                }

                data << KGet::TransferData(sourceUrl, destUrl);
            }

            //all urls have been handled
            urls.clear();
    }

    ///Now handle default folders/groups
    kDebug(5001) << "DIRECTORIES AS SUGGESTION" << Settings::directoriesAsSuggestion();
    if (!Settings::directoriesAsSuggestion() && !urls.isEmpty()) {
        kDebug(5001) << "No, Directories not as suggestion";

        //find the associated groups first, we just need the first matching group though
        const QList<TransferGroupHandler*> groups = KGet::allTransferGroups();
        foreach (TransferGroupHandler *group, groups) {
            if (urls.isEmpty()) {
                break;
            }

            const QString folder = group->defaultFolder();
            if (!KGet::isValidDestDirectory(folder)) {
                continue;
            }

            const QString groupName = group->name();
            const QStringList patterns = group->regExp().pattern().split(',');

            KUrl::List::iterator it = urls.begin();
            while (it != urls.end()) {
                const KUrl sourceUrl = *it;
                if (KGet::matchesExceptions(sourceUrl, patterns)) {
                    const KUrl destUrl = KGet::getValidDestUrl(KUrl(folder), sourceUrl);
                    if (destUrl.isEmpty()) {
                        kWarning(5001) << "Could not create a valid dest url for" << sourceUrl;
                        it = urls.erase(it);
                        continue;
                    }

                    data << KGet::TransferData(sourceUrl, destUrl, groupName);
                    it = urls.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }

    ///Now create transfers for the urls that provided enough data
    if (!data.isEmpty()) {
        KGet::createTransfers(data);
    }

    ///Handle custom newtransferdialogs...
    if ((!m_dialog || m_dialog->isEmpty()) && urls.count() == 1) {//FIXME why the m_dialog check? whenever a dialog has been created this would not be shown?
        KUrl url = urls.first();
        QPointer<KDialog> dialog;
        foreach (TransferFactory * factory, KGet::factories()) {
            const QList<TransferGroupHandler*> groups =  KGet::groupsFromExceptions(url);
            dialog = factory->createNewTransferDialog(url, suggestedFileName, !groups.isEmpty() ? groups.first() : 0);
            if (dialog) {
                if(parentWidget) {
                    KWindowInfo info = KWindowSystem::windowInfo(parentWidget->winId(), NET::WMDesktop, NET::WMDesktop);
                    KWindowSystem::setCurrentDesktop(info.desktop());
                    KWindowSystem::forceActiveWindow(parentWidget->winId());
                }
                dialog->exec();
                delete dialog;
            }
        }
    }

    m_numJobs.remove(jobId);
    m_urls.erase(itUrls);

    ///Display default NewTransferDialog
    if (!urls.isEmpty()) {
        createDialog(urls, suggestedFileName, parentWidget);
    }
}

void NewTransferDialogHandler::createDialog(const KUrl::List &urls, const QString &suggestedFileName, QWidget *parent)
{
    if (!m_dialog) {
        m_dialog = new NewTransferDialog(parent);
    }

    m_dialog->m_window = parent;
    m_dialog->showDialog(urls, suggestedFileName);
}

#include "newtransferdialog.moc"
