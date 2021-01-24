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

#include <QFileDialog>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QTimer>

#include <KLocalizedString>
#include <KMessageBox>

//TODO for 4.4 look at the changes of the newest Draft --> what elements have to be added/removed

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

    if (!urls.isEmpty()) {
        Q_EMIT urlsDropped(urls);
    }
}

MetalinkCreator::MetalinkCreator(QWidget *parent)
  : KAssistantDialog(parent),
    m_needUrlCount(0),
    m_countrySort(nullptr),
    m_languageModel(nullptr),
    m_languageSort(nullptr),
    m_introduction(nullptr),
    m_generalPage(nullptr),
    m_filesModel(nullptr)
{
    create();

    connect(finishButton(), &QPushButton::clicked, this, &MetalinkCreator::slotSave);
    connect(this, &KPageDialog::currentPageChanged, this, &MetalinkCreator::slotUpdateAssistantButtons);

    qRegisterMetaType<KGetMetalink::File>("KGetMetalink::File");
    connect(&m_thread, SIGNAL(fileResult(KGetMetalink::File)), this, SLOT(slotAddFile(KGetMetalink::File)));
    connect(&m_thread, &QThread::finished, this, &MetalinkCreator::slotThreadFinished);

    setWindowTitle(i18n("Create a Metalink"));
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
    backButton()->setEnabled(to != m_generalPage);

    if (!m_filesModel->rowCount()) {
        uiFiles.infoWidget->setText(i18n("Add at least one file."));
    } else if (m_needUrlCount) {
        uiFiles.infoWidget->setText(i18n("You need to set mirrors for the entries with an icon."));
    }
    uiFiles.infoWidget->setVisible(!m_filesModel->rowCount() || m_needUrlCount);

    //only enable finish when the metalink is valid (i.e. no required data missing)
    //and the thread is not running
    finishButton()->setEnabled(metalink.isValid() && !m_thread.isRunning());
}

void MetalinkCreator::create()
{
    createIntroduction();
    m_general = new GeneralWidget(this);
    m_generalPage = addPage(m_general, i18n("General optional information for the metalink."));
    QTimer::singleShot(0, this, &MetalinkCreator::slotDelayedCreation);
}

void MetalinkCreator::slotDelayedCreation()
{
    auto *countryModel = new CountryModel(this);
    countryModel->setupModelData();
    m_countrySort = new QSortFilterProxyModel(this);
    m_countrySort->setSourceModel(countryModel);
    m_countrySort->setSortLocaleAware(true);
    m_countrySort->sort(0);

    m_languageModel = new LanguageModel(this);
    m_languageModel->setupModelData();
    m_languageSort = new QSortFilterProxyModel(this);
    m_languageSort->setSourceModel(m_languageModel);
    m_languageSort->setSortLocaleAware(true);
    m_languageSort->sort(0);

    createFiles();
    slotUpdateIntroductionNextButton();
}

void MetalinkCreator::load()
{
    QUrl url = uiIntroduction.load->url();
    if (uiIntroduction.loadButton->isChecked() && url.isValid())
    {
        if (!KGetMetalink::HandleMetalink::load(url, &metalink))
        {
            KMessageBox::error(this, i18n("Unable to load: %1", url.toString()), i18n("Error"));
        }
    }

    m_general->load(metalink);
    loadFiles();
}

void MetalinkCreator::slotSave()
{
    m_general->save(&metalink);

    QUrl url = uiIntroduction.save->url();
    if (url.isValid())
    {
        if(!KGetMetalink::HandleMetalink::save(url, &metalink))
        {
            KMessageBox::error(this, i18n("Unable to save to: %1", url.toString()), i18n("Error"));
        }
    }
}

void MetalinkCreator::createIntroduction()
{
    auto *widget = new QWidget(this);
    uiIntroduction.setupUi(widget);

    uiIntroduction.save->setFilter("*.meta4|" + i18n("Metalink Version 4.0 file (*.meta4)") + "\n*.metalink|" + i18n("Metalink Version 3.0 file (*.metalink)"));
    uiIntroduction.save->setAcceptMode(QFileDialog::AcceptSave);

    connect(uiIntroduction.save, &KUrlRequester::textChanged, this, &MetalinkCreator::slotUpdateIntroductionNextButton);
    connect(uiIntroduction.load, &KUrlRequester::textChanged, this, &MetalinkCreator::slotUpdateIntroductionNextButton);
    connect(uiIntroduction.loadButton, &QAbstractButton::toggled, this, &MetalinkCreator::slotUpdateIntroductionNextButton);

    m_introduction = addPage(widget, i18n("Define the saving location."));

    setValid(m_introduction, false);
}

void MetalinkCreator::slotUpdateIntroductionNextButton()
{
    bool enableNext = false;

    //check if a save location and if selected if also a load location has been specified and if the m_countrySort has been created
    enableNext = uiIntroduction.save->url().isValid() && m_countrySort;
    if (enableNext && uiIntroduction.loadButton->isChecked()) {
        enableNext = uiIntroduction.load->url().isValid();
    }

    setValid(m_introduction, enableNext);
}

void MetalinkCreator::createFiles()
{
    m_handler = new DirectoryHandler(this);
    connect(m_handler, &DirectoryHandler::finished, this, &MetalinkCreator::slotOpenDragDlg);

    auto *widget = new FileWidget(this);
    uiFiles.setupUi(widget);

    m_filesModel = new QStandardItemModel(0, 1, this);
    uiFiles.files->setModel(m_filesModel);

    uiFiles.infoWidget->setCloseButtonVisible(false);
    uiFiles.infoWidget->setMessageType(KMessageWidget::Information);
    uiFiles.add_local_file->setIcon(QIcon::fromTheme("list-add"));
    KGuiItem::assign(uiFiles.add_file, KStandardGuiItem::add());
    KGuiItem::assign(uiFiles.properties_file, KStandardGuiItem::properties());
    uiFiles.properties_file->setEnabled(false);
    KGuiItem::assign(uiFiles.remove_file, KStandardGuiItem::remove());
    uiFiles.remove_file->setEnabled(false);
    uiFiles.dragDrop->hide();

    connect(uiFiles.add_local_file, &QAbstractButton::clicked, this, &MetalinkCreator::slotAddLocalFilesClicked);
    connect(uiFiles.add_file, &QAbstractButton::clicked, this, &MetalinkCreator::slotAddClicked);
    connect(uiFiles.remove_file, &QAbstractButton::clicked, this, &MetalinkCreator::slotRemoveFile);
    connect(uiFiles.properties_file, &QAbstractButton::clicked, this, &MetalinkCreator::slotFileProperties);
    connect(uiFiles.files->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MetalinkCreator::slotUpdateFilesButtons);
    connect(widget, &FileWidget::urlsDropped, m_handler, &DirectoryHandler::slotFiles);

    addPage(widget, i18nc("file as in file on hard drive", "Files"));
}

void MetalinkCreator::loadFiles()
{
    foreach (const KGetMetalink::File &file, metalink.files.files)
    {
        auto *item = new QStandardItem(file.name);
        if (!file.resources.isValid())
        {
            ++m_needUrlCount;
            item->setIcon(QIcon::fromTheme("edit-delete"));
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
    QPointer<QFileDialog> dialog = new QFileDialog(this);
    dialog->setFileMode(QFileDialog::ExistingFiles);
    if (dialog->exec() == QDialog::Accepted) {
        m_handler->slotFiles(dialog->selectedUrls());
    }
    delete dialog;
}

void MetalinkCreator::slotAddFile()
{
    auto *item = new QStandardItem(m_tempFile.name);
    m_filesModel->insertRow(m_filesModel->rowCount(), item);
    metalink.files.files.append(m_tempFile);
    m_tempFile.clear();

    slotUpdateAssistantButtons(nullptr, m_files);
}

void MetalinkCreator::slotAddFile(const KGetMetalink::File &file)
{
    auto *item = new QStandardItem(file.name);
    if (!file.resources.isValid())
    {
        ++m_needUrlCount;
        item->setIcon(QIcon::fromTheme("edit-delete"));
    }
    m_filesModel->insertRow(m_filesModel->rowCount(), item);
    metalink.files.files.append(file);

    slotUpdateAssistantButtons(nullptr, m_files);
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
        item->setIcon(QIcon());
    }

    slotUpdateAssistantButtons(nullptr, m_files);
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
    slotUpdateAssistantButtons(nullptr, m_files);
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

    auto *fileDlg = new FileDlg(file, currentNames, m_countrySort, m_languageSort, this, edit);
    fileDlg->setAttribute(Qt::WA_DeleteOnClose);
    fileDlg->setWindowModality(Qt::ApplicationModal);
    fileDlg->show();

    connect(fileDlg, SIGNAL(addFile()), this, SLOT(slotAddFile()));
    connect(fileDlg, &FileDlg::fileEdited, this, &MetalinkCreator::slotFileEdited);
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
void MetalinkCreator::slotOpenDragDlg()
{
    m_tempResources.clear();
    m_tempCommonData.clear();
    auto *dragDlg = new DragDlg(&m_tempResources, &m_tempCommonData, m_countrySort, m_languageSort, this);
    dragDlg->setAttribute(Qt::WA_DeleteOnClose);
    dragDlg->show();

    connect(dragDlg, &DragDlg::usedTypes, this, &MetalinkCreator::slotHandleDropped);
}
void MetalinkCreator::slotHandleDropped(const QStringList &types, bool createPartial)
{
    uiFiles.progressBar->setMaximum(0);
    uiFiles.dragDrop->show();
    m_thread.setData(m_handler->takeFiles(), types, createPartial, m_tempResources, m_tempCommonData);
}

void MetalinkCreator::slotThreadFinished()
{
    uiFiles.progressBar->setMaximum(10);
    uiFiles.dragDrop->hide();
    slotUpdateAssistantButtons(nullptr, m_files);
}


