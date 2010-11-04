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
#include "mainwindow.h"
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
#include <QListWidgetItem>
#include <KColorScheme>
#include <KDebug>
#include <KFileDialog>
#include <KWindowSystem>

K_GLOBAL_STATIC(NewTransferDialogHandler, newTransferDialogHandler)


NewTransferDialog::NewTransferDialog(QWidget *parent)
  : KDialog(parent),
    m_window(0),
    m_displayed(false),
    m_multiple(false),
    m_wrongUrl(false)
{
    setModal(true);
    setCaption(i18n("New Download"));
    showButtonSeparator(true);

    QWidget *widget = new QWidget(this);
    ui.setupUi(widget);
    setMainWidget(widget);

    enableButtonOk(false);

    KColorScheme scheme(QPalette::Active, KColorScheme::Window);
    QPalette palette = ui.error->palette();
    palette.setBrush(QPalette::WindowText, scheme.foreground(KColorScheme::NegativeText));
    ui.error->setPalette(palette);


    // properties of the m_destRequester combobox
    ui.destRequester->comboBox()->setDuplicatesEnabled(false);
    ui.destRequester->comboBox()->setUrlDropsEnabled(true);
    ui.destRequester->comboBox()->setEditable(true);
    ui.destRequester->fileDialog()->setKeepLocation(true);

    ui.titleWidget->setPixmap(KIcon("document-new").pixmap(22, 22), KTitleWidget::ImageLeft);

    clear();

    connect(ui.groupComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setDefaultDestination()));

    //TODO ones urlChecker is in do in the following slots the checking if the urls are correct
    //then m_wrongUrl could be removed, as well as redisplaying the dialog
    connect(ui.destRequester, SIGNAL(textChanged(QString)), this, SLOT(destUrlChanged(QString)));
    connect(ui.urlRequester, SIGNAL(textChanged(QString)), this, SLOT(urlChanged(QString)));
}

NewTransferDialog::~NewTransferDialog()
{
}

void NewTransferDialog::setMultiple(bool useMultiple)
{
    m_multiple = useMultiple;

    const Qt::Alignment alignment = Qt::AlignLeft | (m_multiple ? Qt::AlignTop : Qt::AlignVCenter);
    ui.urlLabel->setAlignment(alignment);
    ui.urlRequester->setVisible(!m_multiple);
    ui.listWidget->setVisible(m_multiple);
    ui.destRequester->setMode(m_multiple ? KFile::Directory : KFile::File);
}

void NewTransferDialog::clear()
{
    ui.urlRequester->clear();
    ui.listWidget->clear();
    ui.destRequester->comboBox()->clear();
    ui.destRequester->clear();
    m_displayed = false;
    ui.groupComboBox->clear();

    foreach (TransferGroupHandler *group, KGet::allTransferGroups()) {
        ui.groupComboBox->addItem(KIcon(group->iconName()), group->name());
    }
    ui.groupComboBox->setCurrentIndex(0);
}

KUrl::List NewTransferDialog::sources() const
{
    KUrl::List list;
    if (m_multiple) {
        for (int i = 0; i != ui.listWidget->count(); i++) {
            if (ui.listWidget->item(i)->checkState() == Qt::Checked) {
                list << KUrl(ui.listWidget->item(i)->text().trimmed());
            }
        }
    } else {
        list << KUrl(ui.urlRequester->text().trimmed());
    }

    return list;
}

void NewTransferDialog::setSource(const KUrl::List &sources)
{
    if (sources.isEmpty()) {
        return;
    }

    if (sources.count() == 1) {
        KUrl m_srcUrl = sources.first().url();
        ui.urlRequester->clear();
        if (m_srcUrl.isEmpty())
            m_srcUrl = KUrl(QApplication::clipboard()->text(QClipboard::Clipboard).trimmed());

        //TODO use  urlChecker here once it is done
        if (m_srcUrl.isValid() && !m_srcUrl.protocol().isEmpty())
            ui.urlRequester->insert(m_srcUrl.prettyUrl());
    } else {
        foreach (const KUrl &sourceUrl, sources) {
            if (sourceUrl.url() != KUrl(sourceUrl.url()).fileName()) {//TODO simplify, whatfor is this check anyway, shouldn't the sources be checked already??
                kDebug(5001) << "Insert" << sourceUrl;
                QListWidgetItem *newItem = new QListWidgetItem(sourceUrl.pathOrUrl(), ui.listWidget);
                newItem->setCheckState(Qt::Checked);
            }
        }
    }

    const QList<TransferGroupHandler*> groups = KGet::groupsFromExceptions(sources.first());
    if (!groups.isEmpty()) {
        ui.groupComboBox->setCurrentIndex(ui.groupComboBox->findText(groups.first()->name()));
    }
}

void NewTransferDialog::setDestinationFileName(const QString &filename)
{
    ui.destRequester->setUrl(ui.destRequester->url().path(KUrl::AddTrailingSlash) + filename);
}

//TODO improve this method, so that it does not take sources and has a better name
void NewTransferDialog::setDestination(const KUrl::List &sources, const QStringList &l)
{
    Q_UNUSED(sources)

    ui.destRequester->comboBox()->clear();
    ui.destRequester->clear();

    QStringList list = l;
    kDebug(5001) << list;
    QString filename = destination();

    kDebug(5001) << "Seting destination :multiple=" << m_multiple << " and filename=" << filename;
    if (!m_multiple) {
        filename = KUrl(ui.urlRequester->text().trimmed()).fileName();
    } else if (!filename.isEmpty()) {
        filename = KUrl(filename).directory();
    }

    for (int i = 0; i < list.count(); ++i) {
        if (!list.at(i).endsWith('/'))
            list[i].append('/');
        list[i].append(filename);
    }

    const QString downloadPath = KGet::generalDestDir();
    if (!downloadPath.isEmpty()) {
        list << downloadPath;
    }
    list.removeDuplicates();
    kDebug(5001) << list;

    ui.destRequester->comboBox()->insertItems(0, list);

    //sets destRequester to either display the defaultFolder of group or the generalDestDir
    QString group = ui.groupComboBox->currentText();
    TransferGroupHandler * current = 0;
    foreach (TransferGroupHandler * handler, KGet::allTransferGroups()) {
        if (handler->name() == group) {
            current = handler;
            break;
        }
    }
    if (current && !current->defaultFolder().isEmpty()) {
        if (ui.destRequester->comboBox()->findText(current->defaultFolder()) == -1) {
            ui.destRequester->comboBox()->addItem(current->defaultFolder());
        }
        ui.destRequester->comboBox()->setCurrentIndex(ui.destRequester->comboBox()->findText(current->defaultFolder()));
    } else if (current) {
        ui.destRequester->comboBox()->setCurrentIndex(ui.destRequester->comboBox()->findText(downloadPath));
    }
}

QString NewTransferDialog::destination() const
{
    return ui.destRequester->url().prettyUrl();
}

QString NewTransferDialog::transferGroup() const
{
    return ui.groupComboBox->currentText();
}

void NewTransferDialog::showDialog(const KUrl::List &list, const QString &suggestedFileName)
{
    m_sources << list;

    clear();//Let's clear the old stuff
    kDebug(5001) << "SET SOURCES " << list << " MULTIPLE " << (m_sources.size () > 1);
    setMultiple(m_sources.size() > 1);

    if (!m_sources.isEmpty()) {
        if (list.count() == 1 && !suggestedFileName.isEmpty()) {
            setDestinationFileName(suggestedFileName);
        }

        setSource(m_sources);
    }

    resizeDialog();
    prepareDialog();
}

void NewTransferDialog::setDefaultDestination()
{
    QStringList list;
    foreach (TransferGroupHandler *handler, KGet::allTransferGroups()) {
        if (!handler->defaultFolder().isEmpty())
            list << handler->defaultFolder();
    }
    setDestination(m_sources, list);
    if (!m_sources.isEmpty())
        urlChanged(m_sources.first().path());
    //d->setDestination(m_sources, KGet::defaultFolders(m_sources.first().path(), d->transferGroup()));
}

void NewTransferDialog::prepareDialog()
{
    //only set default destination if the previous operation did not result in an error
    //otherwise the old data should be shown, so that it can be fixed
    if (!m_wrongUrl) {
        setDefaultDestination();
    }
    ui.errorWidget->setVisible(m_wrongUrl);

    if (m_window) {
        KWindowInfo info = KWindowSystem::windowInfo(m_window->winId(), NET::WMDesktop, NET::WMDesktop);
        KWindowSystem::setCurrentDesktop(info.desktop());
        KWindowSystem::forceActiveWindow(m_window->winId());
    }

    if (!m_displayed) {
        m_displayed = true;
        kDebug(5001) << "Exec the dialog!";

        KDialog::exec();

        if (result() == KDialog::Accepted)
        {
            QString destDir = destination();
            m_sources = sources();

            destDir = KUrl(destDir).toLocalFile();

            if (!KGet::isValidDestDirectory(destDir)) {//TODO urlChecker
                kWarning(5001) << "Invalid dest dir, displaying dialog again.";
                ui.errorText->setText(i18n("The specified destination directory is not valid."));
                m_wrongUrl = true;
                m_displayed = false;
                prepareDialog();
                return;
            }

            QString dir;
            if (QFileInfo(destDir).isDir())
                dir = destDir;
            else
                dir = KUrl(destDir).directory();

            Settings::setLastDirectory(dir);
            Settings::self()->writeConfig();

            kDebug(5001) << m_sources;

            QList<KGet::TransferData> data;
            if (m_sources.count() == 1) {
                const KUrl sourceUrl = m_sources.takeFirst();
                if (!KGet::isValidSource(sourceUrl)) {
                    kWarning(5001) << "Could not create a valid dest url for" << sourceUrl;
                    ui.errorText->setText(i18n("The specified source url is not valid."));
                    m_wrongUrl = true;
                    m_displayed = false;
                    prepareDialog();
                    return;
                }

                //false means that the user did not want to download it
                if (KGet::isValidSource(sourceUrl)) {
                    //empty means that the user aborted it, e.g. because he did not want to overwrite it
                    const KUrl destUrl = KGet::getValidDestUrl(KUrl(destDir), sourceUrl);
                    if (!destUrl.isEmpty()) {
                        data << KGet::TransferData(sourceUrl, destUrl, transferGroup());
                    }
                }
            } else {
                foreach (const KUrl &sourceUrl, m_sources) {
                    //empty means that the user aborted it, e.g. because he did not want to overwrite it
                    const KUrl destUrl = KGet::getValidDestUrl(KUrl(destDir), sourceUrl);
                    if (!destUrl.isEmpty()) {
                        data << KGet::TransferData(sourceUrl, destUrl, transferGroup());
                    }
                }
            }

            if (!data.isEmpty()) {
                KGet::createTransfers(data);
            }
        }
        m_wrongUrl = false;

        m_sources.clear();
        clear();
    }
}

/**
 * FIXME
 * The dialog is always lagging behind, i.e. if first m_multiple is false, then a small dialog will be displayed
 * if next m_multiple is true then still a small dialog will be displayed, then next dialog displayed no matter
 * if m_multiple is true will be large
*/
void NewTransferDialog::resizeDialog()
{
    if (KGet::transferGroupNames().count() < 2) {
        ui.groupComboBox->hide();
        ui.groupLabel->hide();
    }
}

bool NewTransferDialog::isEmpty()
{
    return (m_multiple ? !ui.listWidget->count() : ui.urlRequester->text().trimmed().isEmpty());
}

void NewTransferDialog::urlChanged(const QString &text)
{
    if (m_multiple) {
        return;
    }

    const QString destUrl = ui.destRequester->text();
    KUrl url(text.trimmed());
    //if (d->m_destRequester->url()->isEmpty())
    //    d->setDestination(m_sources, QStringList());
    if (QFileInfo(ui.destRequester->url().toLocalFile()).isDir())
        setDestinationFileName(url.fileName());

    enableButtonOk(!text.isEmpty() && !destUrl.isEmpty());
    kDebug() << url << url.fileName() << ui.destRequester->url().fileName();
}

void NewTransferDialog::destUrlChanged(const QString &url)
{
    if (m_multiple) {
        enableButtonOk(!url.isEmpty());
    } else {
        enableButtonOk(!ui.urlRequester->text().isEmpty() && !url.isEmpty());
    }
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
}


void NewTransferDialogHandler::showNewTransferDialog(const KUrl &url)
{
    showNewTransferDialog(url.isEmpty() ? KUrl::List() : KUrl::List() << url);
}

void NewTransferDialogHandler::showNewTransferDialog(KUrl::List urls)
{
    if (urls.isEmpty()) {
        newTransferDialogHandler->createDialog(urls, QString());
        return;
    }

    QHash<int, UrlData>::iterator itUrls = newTransferDialogHandler->m_urls.insert(newTransferDialogHandler->m_nextJobId, UrlData());
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
        kWarning(5001) << "An error happened for" << job->url();
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

    KUrl::List::iterator it = urls.begin();
    while (it != urls.end()) {
        //TODO urlChecker + multiple questions at ones
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
                KWindowInfo info = KWindowSystem::windowInfo(KGet::m_mainWindow->winId(), NET::WMDesktop, NET::WMDesktop);
                KWindowSystem::setCurrentDesktop(info.desktop());
                KWindowSystem::forceActiveWindow(KGet::m_mainWindow->winId());

                dialog->exec();
                delete dialog;
            }
        }
    }

    m_numJobs.remove(jobId);
    m_urls.erase(itUrls);

    ///Display default NewTransferDialog
    if (!urls.isEmpty()) {
        createDialog(urls, suggestedFileName);
    }
}

void NewTransferDialogHandler::createDialog(const KUrl::List &urls, const QString &suggestedFileName)
{
    if (!m_dialog) {
        m_dialog = new NewTransferDialog(KGet::m_mainWindow);
    }

    m_dialog->m_window = KGet::m_mainWindow;
    m_dialog->showDialog(urls, suggestedFileName);
}

#include "newtransferdialog.moc"
