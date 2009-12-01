/***************************************************************************
*   Copyright (C) 2009 Matthias Fuchs <mat69@gmx.net>                     *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
***************************************************************************/

#include "metalinkcreator.h"
#include "filedlg.h"
#include "dragdlg.h"
#include "localemodels.h"
#include "generalwidget.h"

#include "core/verifier.h"

#include <QtGui/QDragEnterEvent>
#include <QtGui/QLabel>
#include <QtGui/QSortFilterProxyModel>
#include <QtGui/QStandardItemModel>

#include <KFileDialog>
#include <KLocale>
#include <KMessageBox>
#include <KPushButton>
#include <KStandardDirs>

//TODO for 4.4 look at the changes of the newest Draft --> what elements have to be added/removed

DroppedFilesThread::DroppedFilesThread(QObject *parent)
  : QThread(parent),
    abort(false)
{
}

DroppedFilesThread::~DroppedFilesThread()
{
    mutex.lock();
    abort = true;
    mutex.unlock();

    wait();
}

void DroppedFilesThread::setData(const QList<KUrl> &files, const QStringList &types, bool createPartial, const KGetMetalink::Resources &tempResources, const KGetMetalink::CommonData &tempCommonData)
{
    QMutexLocker locker(&mutex);
    m_files.append(files);
    m_types.append(types);
    m_createPartial.append(createPartial);
    m_tempResources.append(tempResources);
    m_tempCommonDatas.append(tempCommonData);

    if (!isRunning())
    {
        start();
    }
}

void DroppedFilesThread::run()
{
    mutex.lock();
    bool run = m_files.count();
    mutex.unlock();

    while (run && !abort)
    {
        mutex.lock();
        QList<KUrl> files = m_files.takeFirst();
        QStringList types = m_types.takeFirst();
        bool createPartial = m_createPartial.takeFirst();
        KGetMetalink::Resources tempResources = m_tempResources.takeFirst();
        KGetMetalink::CommonData commonData = m_tempCommonDatas.takeFirst();
        mutex.unlock();

        while (files.count() && !abort)
        {
            //take the first file and try to handle it
            const KUrl url = files.takeFirst();
            KGetMetalink::File file;
    //         //see if the file has been already added, in that case modify it//TODO
    //         for (int i = 0; i < metalink.files.files.count(); ++i)
    //         {
    //             if (metalink.files.files[i].name == url.fileName())
    //             {
    //                 file = &metalink.files.files[i];
    //                 update = true;
    //                 break;
    //             }
    //         }

            file.name = url.fileName();
            QFile localFile(url.path());
            file.size = localFile.size();
            file.data = commonData;

            foreach (const KGetMetalink::Url &metalinkUrl, tempResources.urls)
            {
                KGetMetalink::Url mirror = metalinkUrl;
                mirror.url.addPath(file.name);

                //if the url has already been added, remove it and readd it
                for (int i = 0; i < file.resources.urls.count(); ++i)
                {
                    if (file.resources.urls[i].url == mirror.url)
                    {
                        file.resources.urls.removeAt(i);
                        break;
                    }
                }
                file.resources.urls.append(mirror);
            }

            foreach (const QString &type, types)
            {
                if (abort)
                {
                    return;
                }

                QString hash = Verifier::checksum(url, type, &abort);
                if (!hash.isEmpty())
                {
                    file.verification.hashes[type] = hash;
                }
            }

            if (createPartial)
            {
                foreach (const QString &type, types)
                {
                    if (abort)
                    {
                        return;
                    }

                    PartialChecksums partialChecksums = Verifier::partialChecksums(url, type, 0, &abort);
                    if (partialChecksums.isValid())
                    {
                        KGetMetalink::Pieces pieces;
                        pieces.type = type;
                        pieces.length = partialChecksums.length();
                        pieces.hashes = partialChecksums.checksums();
                        file.verification.pieces.append(pieces);
                    }
                }
            }
            if (!abort)
            {
                emit fileResult(file);
            }
        }
        mutex.lock();
        run = m_files.count();
        mutex.unlock();
    }
}


FileWidget::FileWidget(QWidget *parent)
  : QWidget(parent)
{
    setAcceptDrops(true);
}


void FileWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls())
    {
        event->acceptProposedAction();
    }
}

void FileWidget::dropEvent(QDropEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();

    event->acceptProposedAction();

    QList<KUrl> kurls;

    foreach (const QUrl &url, urls)
    {
        QDir dir(url.path());
        //TODO implement directories
        if (dir.exists())
        {
            const QStringList files = dir.entryList(QDir::Files);
        }
        else
        {
            kurls.append(url);
        }
    }

    if (kurls.count())
    {
        emit urlsDropped(kurls);
    }
}

MetalinkCreator::MetalinkCreator(QWidget *parent)
  : KAssistantDialog(parent),
    m_needUrlCount(0),
    m_introduction(0),
    m_generalPage(0),
    m_filesModel(0)
{
    CountryModel *countryModel = new CountryModel(this);
    countryModel->setupModelData(KGlobal::locale()->allCountriesList());
    m_countrySort = new QSortFilterProxyModel(this);
    m_countrySort->setSourceModel(countryModel);
    m_countrySort->sort(0);

    m_languageModel = new LanguageModel(this);
    m_languageModel->setupModelData(KGlobal::locale()->allLanguagesList());
    m_languageSort = new QSortFilterProxyModel(this);
    m_languageSort->setSourceModel(m_languageModel);
    m_languageSort->sort(0);

    create();

    connect(this, SIGNAL(user1Clicked()), this, SLOT(slotSave()));
    connect(this, SIGNAL(currentPageChanged(KPageWidgetItem*,KPageWidgetItem*)), this, SLOT(slotUpdateAssistantButtons(KPageWidgetItem*,KPageWidgetItem*)));

    qRegisterMetaType<KGetMetalink::File>("KGetMetalink::File");
    connect(&thread, SIGNAL(fileResult(const KGetMetalink::File &)), this, SLOT(slotAddFile(const KGetMetalink::File &)));
    connect(&thread, SIGNAL(finished()), this, SLOT(slotThreadFinished()));

    setCaption(i18n("Create a Metalink"));
    showButton(KDialog::Help, false);
}

MetalinkCreator::~MetalinkCreator()
{
}

void MetalinkCreator::slotUpdateAssistantButtons(KPageWidgetItem *to, KPageWidgetItem *from)
{
    //once we leave the introduction page the data is being loaded
    if (m_introduction && m_generalPage && (from == m_introduction) && (to == m_generalPage))
    {
        load();
    }

    //it is impossible to return to the introduction page
    if (to == m_generalPage)
    {
        enableButton(KDialog::User3, false);
    }
    else
    {
        enableButton(KDialog::User3, true);
    }

    uiFiles.needUrl->setVisible(m_needUrlCount);

    //only enable finish then the metalink is valid (i.e. no required data missing)
    //and the thread is not running
    enableButton(KDialog::User1, metalink.isValid() && !thread.isRunning());
}

void MetalinkCreator::create()
{
    createIntroduction();
    m_general = new GeneralWidget(this);
    m_generalPage = addPage(m_general, i18n("General optional information for the metalink."));
    createFiles();
}

void MetalinkCreator::load()
{
    KUrl url = KUrl(uiIntroduction.load->text());
    if (uiIntroduction.loadButton->isChecked() && url.isValid())
    {
        if (!KGetMetalink::HandleMetalink::load(url, &metalink))
        {
            KMessageBox::error(this, i18n("Unable to load: %1", url.pathOrUrl()), i18n("Error"));
        }
    }

    m_general->load(metalink);
    loadFiles();
}

void MetalinkCreator::slotSave()
{
    m_general->save(&metalink);

    KUrl url = KUrl(uiIntroduction.save->text());
    if (url.isValid())
    {
        if(!KGetMetalink::HandleMetalink::save(url, &metalink))
        {
            KMessageBox::error(this, i18n("Unable to save to: %1", url.pathOrUrl()), i18n("Error"));
        }
    }
}

void MetalinkCreator::createIntroduction()
{
    QWidget *widget = new QWidget(this);
    uiIntroduction.setupUi(widget);

    uiIntroduction.save->setFilter("*.meta4|" + i18n("Metalink Version 4.0 file (*.meta4)") + "\n*.metalink|" + i18n("Metalink Version 3.0 file (*.metalink)"));

    connect(uiIntroduction.save, SIGNAL(textChanged(QString)), this, SLOT(slotUpdateIntroductionNextButton()));
    connect(uiIntroduction.load, SIGNAL(textChanged(QString)), this, SLOT(slotUpdateIntroductionNextButton()));
    connect(uiIntroduction.loadButton, SIGNAL(toggled(bool)), this, SLOT(slotUpdateIntroductionNextButton()));

    m_introduction = addPage(widget, i18n("Define the saving location."));

    setValid(m_introduction, false);
}

void MetalinkCreator::slotUpdateIntroductionNextButton()
{
    bool enableNext = false;

    //check if a save location and if selected if also a load location has been specified
    enableNext = uiIntroduction.save->url().isValid();
    if (enableNext && uiIntroduction.loadButton->isChecked()) {
        enableNext = uiIntroduction.load->url().isValid();
    }

    setValid(m_introduction, enableNext);
}

void MetalinkCreator::createFiles()
{
    FileWidget *widget = new FileWidget(this);
    uiFiles.setupUi(widget);

    m_filesModel = new QStandardItemModel(0, 1, this);
    uiFiles.files->setModel(m_filesModel);

    uiFiles.add_local_file->setIcon(KIcon("list-add"));
    uiFiles.add_file->setGuiItem(KStandardGuiItem::add());
    uiFiles.properties_file->setGuiItem(KStandardGuiItem::properties());
    uiFiles.properties_file->setEnabled(false);
    uiFiles.remove_file->setGuiItem(KStandardGuiItem::remove());
    uiFiles.remove_file->setEnabled(false);
    uiFiles.needUrl->hide();
    uiFiles.dragDrop->hide();

    connect(uiFiles.add_local_file, SIGNAL(clicked(bool)), this, SLOT(slotAddLocalFilesClicked()));
    connect(uiFiles.add_file, SIGNAL(clicked(bool)), this, SLOT(slotAddClicked()));
    connect(uiFiles.remove_file, SIGNAL(clicked(bool)), this, SLOT(slotRemoveFile()));
    connect(uiFiles.properties_file, SIGNAL(clicked(bool)), this, SLOT(slotFileProperties()));
    connect(uiFiles.files->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(slotUpdateFilesButtons()));
    connect(widget, SIGNAL(urlsDropped(QList<KUrl>)), this, SLOT(slotUrlsDropped(QList<KUrl>)));

    addPage(widget, i18nc("file as in file on hard drive", "Enter at least one file."));
}

void MetalinkCreator::loadFiles()
{
    foreach (const KGetMetalink::File &file, metalink.files.files)
    {
        QStandardItem *item = new QStandardItem(file.name);
        if (!file.resources.isValid())
        {
            ++m_needUrlCount;
            item->setIcon(KIcon("edit-delete"));
        }
        m_filesModel->insertRow(m_filesModel->rowCount(), item);
    }
}

void MetalinkCreator::slotUpdateFilesButtons()
{
    const QModelIndexList indexes = uiFiles.files->selectionModel()->selectedRows();
    uiFiles.remove_file->setEnabled(indexes.count());

    bool propertiesEnabled = (indexes.count() == 1);
    uiFiles.properties_file->setEnabled(propertiesEnabled);
}

void MetalinkCreator::slotAddLocalFilesClicked()
{
    QPointer<KFileDialog> dialog = new KFileDialog(KUrl(), QString(), this);
    dialog->setMode(KFile::Files | KFile::ExistingOnly | KFile::LocalOnly);
    if (dialog->exec() == QDialog::Accepted)
    {
        slotUrlsDropped(dialog->selectedUrls());
    }
    delete dialog;
}

void MetalinkCreator::slotAddFile()
{
    QStandardItem *item = new QStandardItem(m_tempFile.name);
    m_filesModel->insertRow(m_filesModel->rowCount(), item);
    metalink.files.files.append(m_tempFile);
    m_tempFile.clear();

    slotUpdateAssistantButtons(0, m_files);
}

void MetalinkCreator::slotAddFile(const KGetMetalink::File &file)
{
    QStandardItem *item = new QStandardItem(file.name);
    if (!file.resources.isValid())
    {
        ++m_needUrlCount;
        item->setIcon(KIcon("edit-delete"));
    }
    m_filesModel->insertRow(m_filesModel->rowCount(), item);
    metalink.files.files.append(file);

    slotUpdateAssistantButtons(0, m_files);
}

void MetalinkCreator::slotFileEdited(const QString &oldFileName, const QString &newFileName)
{
    Q_UNUSED(oldFileName)

    const QModelIndex index = uiFiles.files->selectionModel()->selectedRows().first();
    QStandardItem *item = m_filesModel->itemFromIndex(index);
    item->setText(newFileName);

    //had no url but has it now
    if (!item->icon().isNull())
    {
        --m_needUrlCount;
        item->setIcon(KIcon());
    }

    slotUpdateAssistantButtons(0, m_files);
}

void MetalinkCreator::slotRemoveFile()
{
    while (uiFiles.files->selectionModel()->hasSelection()) {
        const QModelIndex index = uiFiles.files->selectionModel()->selectedRows().first();
        const QString filePath = index.data().toString();
        for (int i = 0; i < metalink.files.files.size(); ++i)
        {
            if (metalink.files.files.at(i).name == filePath)
            {
                //the entry had not url, so do not count it anymore
                if (!index.data(Qt::DecorationRole).isNull())
                {
                    --m_needUrlCount;
                }
                metalink.files.files.removeAt(i);
                break;
            }
        }

        m_filesModel->removeRow(index.row());
    }

    slotUpdateFilesButtons();
    slotUpdateAssistantButtons(0, m_files);
}

void MetalinkCreator::slotAddClicked()
{
    //no old stored data should be used
    m_tempFile.clear();
    fileDlg(&m_tempFile);
}

void MetalinkCreator::fileDlg(KGetMetalink::File *file, bool edit)
{
    QStringList currentNames;
    for (int i = 0; i < m_filesModel->rowCount(); ++i)
    {
        currentNames.append(m_filesModel->index(i, 0).data().toString());
    }

    FileDlg *fileDlg = new FileDlg(file, currentNames, m_countrySort, m_languageSort, this, edit);
    fileDlg->setAttribute(Qt::WA_DeleteOnClose);
    fileDlg->setWindowModality(Qt::ApplicationModal);
    fileDlg->show();

    connect(fileDlg, SIGNAL(addFile()), this, SLOT(slotAddFile()));
    connect(fileDlg, SIGNAL(fileEdited(QString,QString)), this, SLOT(slotFileEdited(QString,QString)));
}

void MetalinkCreator::slotFileProperties()
{
    const QModelIndex index = uiFiles.files->selectionModel()->selectedRows().first();
    const QString fileName = index.data().toString();

    //search the selected file in metalink
    for (int i = 0; i < metalink.files.files.count(); ++i)
    {
        if (metalink.files.files.at(i).name == fileName)
        {
            fileDlg(&metalink.files.files[i], true);
            break;
        }
    }
}

void MetalinkCreator::slotUrlsDropped(const QList<KUrl> &files)
{
    m_tempResources.clear();
    m_tempCommonData.clear();
    m_droppedFiles = files;
    DragDlg *dragDlg = new DragDlg(&m_tempResources, &m_tempCommonData, m_countrySort, m_languageSort, this);
    dragDlg->setAttribute(Qt::WA_DeleteOnClose);
    dragDlg->show();

    connect(dragDlg, SIGNAL(usedTypes(QStringList, bool)), this, SLOT(slotHandleDropped(QStringList, bool)));
}

void MetalinkCreator::slotHandleDropped(const QStringList& types, bool createPartial)
{
    uiFiles.progressBar->setMaximum(0);
    uiFiles.dragDrop->show();
    thread.setData(m_droppedFiles, types, createPartial, m_tempResources, m_tempCommonData);
}

void MetalinkCreator::slotThreadFinished()
{
    uiFiles.progressBar->setMaximum(10);
    uiFiles.dragDrop->hide();
    slotUpdateAssistantButtons(0, m_files);
}

#include "metalinkcreator.moc"
