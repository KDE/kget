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
#include <KSystemTimeZone>

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

void DroppedFilesThread::setData(const QList<KUrl> &files, const QStringList &types, bool createPartial, const KGetMetalink::Resources &tempResources)
{
    QMutexLocker locker(&mutex);
    m_files.append(files);
    m_types.append(types);
    m_createPartial.append(createPartial);
    m_tempResources.append(tempResources);

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
        mutex.unlock();

        while (files.count())
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
                QString hash = Verifier::checksum(url, type);
                if (!hash.isEmpty())
                {
                    file.verification.hashes[type] = hash;
                }
            }

            if (createPartial)
            {
                foreach (const QString &type, types)
                {
                    PartialChecksums partialChecksums = Verifier::partialChecksums(url, type);
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
            emit fileResult(file);
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
    m_general(0),
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
}

MetalinkCreator::~MetalinkCreator()
{
}

void MetalinkCreator::slotUpdateAssistantButtons(KPageWidgetItem *to, KPageWidgetItem *from)
{
    //once we leave the introduction page the data is being loaded
    if (m_introduction && m_general && (from == m_introduction) && (to == m_general))
    {
        load();
    }

    //it is impossible to return to the introduction page
    if (to == m_general)
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
    createGeneral();
    createGeneral2();
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

    loadGeneral();
    loadGeneral2();
    loadFiles();
}

void MetalinkCreator::slotSave()
{
    saveGeneral();
    saveGeneral2();

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

    uiIntroduction.save->setMode(KFile::File | KFile::LocalOnly);
    uiIntroduction.save->setFilter("*.metalink|" + i18n("Metalink file"));

    uiIntroduction.createButton->toggle();
    uiIntroduction.load->setEnabled(false);
    uiIntroduction.load->setMode(KFile::File | KFile::LocalOnly | KFile::ExistingOnly);
    uiIntroduction.load->setFilter("*.metalink|" + i18n("Metalink file"));

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
    if (enableNext && uiIntroduction.loadButton->isChecked())
    {
        enableNext = uiIntroduction.load->url().isValid();
    }

    setValid(m_introduction, enableNext);
}

void MetalinkCreator::createGeneral()
{
    QWidget *widget = new QWidget(this);
    uiGeneral.setupUi(widget);

    uiGeneral.dynamic->setToolTip(uiGeneral.labelDynamic->toolTip());
    uiGeneral.language->setModel(m_languageSort);

    uiGeneral.publishedoffset->setEnabled(false);
    uiGeneral.publishedWidget->setEnabled(false);
    uiGeneral.updatedoffset->setEnabled(false);
    uiGeneral.updatedWidget->setEnabled(false);

    m_general = addPage(widget, i18n("General optional information for the metalink."));
}

void MetalinkCreator::loadGeneral()
{
    uiGeneral.origin->setUrl(metalink.origin);
    uiGeneral.identity->setText(metalink.files.data.identity);
    uiGeneral.version->setText(metalink.files.data.version);
    uiGeneral.description->setText(metalink.files.data.description);
    uiGeneral.logo->setUrl(metalink.files.data.logo);
    uiGeneral.os->setText(metalink.files.data.os);

    uiGeneral.dynamic->setChecked(metalink.dynamic);

    const int indexLanguage = uiGeneral.language->findData(metalink.files.data.language);
    uiGeneral.language->setCurrentIndex(indexLanguage);

    uiGeneral.use_published->setChecked(metalink.published.isValid());
    uiGeneral.use_publishedtimeoffset->setChecked(metalink.published.timeZoneOffset.isValid());
    if (metalink.published.isValid())
    {
      uiGeneral.published->setDateTime(metalink.published.dateTime);
      uiGeneral.publishedoffset->setTime(metalink.published.timeZoneOffset);
      uiGeneral.publishedNegative->setChecked(metalink.published.negativeOffset);
    }
    else
    {
      uiGeneral.published->setDateTime(QDateTime::currentDateTime());
      int offset = KSystemTimeZones::local().currentOffset();
      bool negativeOffset = (offset < 0);
      uiGeneral.publishedNegative->setChecked(negativeOffset);
      offset = abs(offset);
      QTime time = QTime(0, 0, 0);
      time = time.addSecs(abs(offset));
      uiGeneral.publishedoffset->setTime(time);
      uiGeneral.use_publishedtimeoffset->setChecked(true);
      uiGeneral.publishedNegative->setChecked(negativeOffset);
    }

    uiGeneral.use_updated->setChecked(metalink.updated.isValid());
    uiGeneral.use_updatedtimeoffset->setChecked(metalink.updated.timeZoneOffset.isValid());
    if (metalink.updated.isValid())
    {
      uiGeneral.updated->setDateTime(metalink.updated.dateTime);
      uiGeneral.updatedoffset->setTime(metalink.updated.timeZoneOffset);
      uiGeneral.updatedNegative->setChecked(metalink.updated.negativeOffset);
    }
    else
    {
      uiGeneral.updated->setDateTime(QDateTime::currentDateTime());
      int offset = KSystemTimeZones::local().currentOffset();
      bool negativeOffset = (offset < 0);
      uiGeneral.updatedNegative->setChecked(negativeOffset);
      QTime time = QTime(0, 0, 0);
      time = time.addSecs(abs(offset));
      uiGeneral.updatedoffset->setTime(time);
      uiGeneral.use_updatedtimeoffset->setChecked(true);
      uiGeneral.updatedNegative->setChecked(negativeOffset);
    }
}

void MetalinkCreator::saveGeneral()
{
    metalink.origin = KUrl(uiGeneral.origin->text());
    metalink.files.data.identity = uiGeneral.identity->text();
    metalink.files.data.version = uiGeneral.version->text();
    metalink.files.data.description = uiGeneral.description->text();
    metalink.files.data.logo = KUrl(uiGeneral.logo->text());
    metalink.files.data.os = uiGeneral.os->text();
    metalink.files.data.identity = uiGeneral.identity->text();

    metalink.dynamic = uiGeneral.dynamic->isChecked();

    metalink.files.data.language = uiGeneral.language->itemData(uiGeneral.language->currentIndex()).toString();

    metalink.published.clear();
    if (uiGeneral.use_published->isChecked())
    {
        metalink.published.dateTime = uiGeneral.published->dateTime();
        if (uiGeneral.use_publishedtimeoffset->isChecked())
        {
            metalink.published.timeZoneOffset = uiGeneral.publishedoffset->time();
        }
    }

    metalink.updated.clear();
    if (uiGeneral.use_updated->isChecked())
    {
        metalink.updated.dateTime = uiGeneral.updated->dateTime();
        if (uiGeneral.use_updatedtimeoffset->isChecked())
        {
            metalink.updated.timeZoneOffset = uiGeneral.updatedoffset->time();
        }
    }
}

void MetalinkCreator::createGeneral2()
{
    QWidget *widget = new QWidget(this);
    uiGeneral2.setupUi(widget);

    m_general2 = addPage(widget, i18n("General optional information for the metalink."));
}

void MetalinkCreator::loadGeneral2()
{
    uiGeneral2.copyright->setText(metalink.files.data.copyright);
    uiGeneral2.lic_name->setText(metalink.files.data.license.name);
    uiGeneral2.lic_url->setUrl(metalink.files.data.license.url);
    uiGeneral2.pub_name->setText(metalink.files.data.publisher.name);
    uiGeneral2.pub_url->setUrl(metalink.files.data.publisher.url);
}

void MetalinkCreator::saveGeneral2()
{
    metalink.files.data.copyright = uiGeneral2.copyright->text();
    metalink.files.data.license.name = uiGeneral2.lic_name->text();
    metalink.files.data.license.url = KUrl(uiGeneral2.lic_url->text());
    metalink.files.data.publisher.name = uiGeneral2.pub_name->text();
    metalink.files.data.publisher.url = KUrl(uiGeneral2.pub_url->text());
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

    connect(uiFiles.add_local_file, SIGNAL(clicked(bool)), this, SLOT(slotAddLocalFilesPressed()));
    connect(uiFiles.add_file, SIGNAL(clicked(bool)), this, SLOT(slotAddPressed()));
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
            item->setIcon(KIcon("text-html"));
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

void MetalinkCreator::slotAddLocalFilesPressed()
{
    KFileDialog *dialog = new KFileDialog(KUrl(), QString(), this);
    dialog->setMode(KFile::Files | KFile::ExistingOnly | KFile::LocalOnly);
    if (dialog->exec() == QDialog::Accepted)
    {
        slotUrlsDropped(dialog->selectedUrls());
    }
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
        item->setIcon(KIcon("text-html"));
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
    const QModelIndexList indexes = uiFiles.files->selectionModel()->selectedRows();
    foreach (const QModelIndex &index, indexes)
    {
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

void MetalinkCreator::slotAddPressed()
{
    //no old stored data should be used
    m_tempFile.clear();
    fileDlg(&m_tempFile);
}

void MetalinkCreator::fileDlg(KGetMetalink::File* file, bool edit)
{
    QStringList currentNames;
    for (int i = 0; i < m_filesModel->rowCount(); ++i)
    {
        currentNames.append(m_filesModel->index(i, 0).data().toString());
    }

    FileDlg *fileDlg = new FileDlg(file, currentNames, m_countrySort, m_languageSort, this, edit);
    fileDlg->setWindowModality(Qt::ApplicationModal);
    fileDlg->show();

    connect(fileDlg, SIGNAL(addFile()), this, SLOT(slotAddFile()));
    connect(fileDlg, SIGNAL(fileEdited(QString,QString)), this, SLOT(slotFileEdited(QString,QString)));
    connect(fileDlg, SIGNAL(finished()), fileDlg, SLOT(deleteLater()));
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
    m_droppedFiles = files;
    DragDlg *dragDlg = new DragDlg(&m_tempResources, m_countrySort, this);
    dragDlg->show();

    connect(dragDlg, SIGNAL(usedTypes(QStringList, bool)), this, SLOT(slotHandleDropped(QStringList, bool)));
    connect(dragDlg, SIGNAL(finished()), dragDlg, SLOT(deleteLater()));
}

void MetalinkCreator::slotHandleDropped(const QStringList& types, bool createPartial)
{
    uiFiles.progressBar->setMaximum(0);
    uiFiles.dragDrop->show();
    thread.setData(m_droppedFiles, types, createPartial, m_tempResources);
}

void MetalinkCreator::slotThreadFinished()
{
    uiFiles.progressBar->setMaximum(10);
    uiFiles.dragDrop->hide();
    slotUpdateAssistantButtons(0, m_files);
}

#include "metalinkcreator.moc"
