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

#include "filedlg.h"
#include "metalinker.h"
#include "urlwidget.h"
#include "../verificationdialog.h"

#include "../../core/verifier.h"
#include "../../core/verificationmodel.h"
#include "../../core/verificationdelegate.h"

#include <QtGui/QSortFilterProxyModel>

#include <KLocale>

FileDlg::FileDlg(KGetMetalink::File *file, const QStringList &currentFileNames, QSortFilterProxyModel *countrySort, QSortFilterProxyModel *languageSort, QWidget *parent, bool edit)
  : KGetSaveSizeDialog("FileDlg", parent),
    m_file(file),
    m_initialFileName(m_file->name),
    m_currentFileNames(currentFileNames),
    m_edit(edit)
{
    //remove the initial name, to see later if the chosen name is still free
    m_currentFileNames.removeAll(m_initialFileName);

    QWidget *widget = new QWidget(this);
    ui.setupUi(widget);
    setMainWidget(widget);

    m_urlWidget = new UrlWidget(this);
    m_urlWidget->init(&m_file->resources, countrySort);
    ui.urlLayout->addWidget(m_urlWidget->widget());
    connect(m_urlWidget, SIGNAL(urlsChanged()), this, SLOT(slotUpdateOkButton()));

    QWidget *data = new QWidget(this);
    uiData.setupUi(data);
    ui.dataLayout->addWidget(data);

    ui.infoWidget->setCloseButtonVisible(false);
    ui.infoWidget->setMessageType(KMessageWidget::Information);

    //set the file data
    ui.name->setText(m_file->name);
    uiData.identity->setText(m_file->data.identity);
    uiData.version->setText(m_file->data.version);
    uiData.description->setText(m_file->data.description);
    uiData.logo->setUrl(m_file->data.logo);
    if (m_file->data.oses.count()) {
        uiData.os->setText(m_file->data.oses.join(i18nc("comma, to seperate members of a list", ",")));
    }
    uiData.copyright->setText(m_file->data.copyright);
    uiData.pub_name->setText(m_file->data.publisher.name);
    uiData.pub_url->setUrl(m_file->data.publisher.url);

    if (m_file->size) {
        ui.size->setText(QString::number(m_file->size));
    }


    //create the language selection
    uiData.language->setModel(languageSort);
    if (m_file->data.languages.count()) {//TODO 4.5 support multiple languages
        const int index = uiData.language->findData(m_file->data.languages.first());
        uiData.language->setCurrentIndex(index);
    } else {
        //Do not select a language of a file if none was defined.
        uiData.language->setCurrentIndex(-1);
    }


    //create the verification stuff
    m_verificationModel = new VerificationModel(this);
    QHash<QString, QString>::const_iterator it;
    QHash<QString, QString>::const_iterator itEnd = m_file->verification.hashes.constEnd();
    for (it = m_file->verification.hashes.constBegin(); it != itEnd; ++it) {
        m_verificationModel->addChecksum(it.key(), it.value());
    }
    ui.add_hash->setGuiItem(KStandardGuiItem::add());
    ui.remove_hash->setGuiItem(KStandardGuiItem::remove());
    m_verificationProxy = new QSortFilterProxyModel(this);
    m_verificationProxy->setSourceModel(m_verificationModel);
    ui.used_hashes->setSortingEnabled(true);
    ui.used_hashes->hideColumn(VerificationModel::Verified);
    ui.used_hashes->setModel(m_verificationProxy);
    ui.used_hashes->setItemDelegate(new VerificationDelegate(this));
    slotUpdateVerificationButtons();

    connect(m_verificationModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(slotUpdateVerificationButtons()));
    connect(m_verificationModel, SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(slotUpdateVerificationButtons()));
    connect(ui.used_hashes, SIGNAL(clicked(QModelIndex)), this, SLOT(slotUpdateVerificationButtons()));
    connect(ui.add_hash, SIGNAL(clicked()), this, SLOT(slotAddHash()));
    connect(ui.remove_hash, SIGNAL(clicked()), this, SLOT(slotRemoveHash()));
    connect(ui.name, SIGNAL(textEdited(QString)), this, SLOT(slotUpdateOkButton()));
    connect(this, SIGNAL(okClicked()), this, SLOT(slotOkClicked()));

    slotUpdateOkButton();

    setCaption(i18n("File Properties"));
}

void FileDlg::slotUpdateOkButton()
{
    bool hasName = !ui.name->text().isEmpty();
    bool hasUrls =  m_urlWidget->hasUrls();
    bool isDuplicate = (m_currentFileNames.indexOf(ui.name->text()) > -1);

    QStringList information;

    if (!hasName) {
        information << i18n("Enter a filename.");
    }
    if (isDuplicate) {
        information << i18n("The filename exists already, choose a different one.");
    }
    if (!hasUrls) {
        information << i18n("Enter at least one URL.");
    }

    ui.infoWidget->setText(information.join(" "));
    ui.infoWidget->setVisible(!information.isEmpty());

    enableButtonOk(hasName && hasUrls && !isDuplicate);
}

void FileDlg::slotUpdateVerificationButtons()
{
    ui.remove_hash->setEnabled(ui.used_hashes->selectionModel()->hasSelection());
}

void FileDlg::slotRemoveHash()
{
    while (ui.used_hashes->selectionModel()->hasSelection()) {
        const QModelIndex index = ui.used_hashes->selectionModel()->selectedRows().first();
        m_verificationModel->removeRow(m_verificationProxy->mapToSource(index).row());
    }
}

void FileDlg::slotAddHash()
{
    VerificationAddDlg *dialog = new VerificationAddDlg(m_verificationModel, this);
    dialog->show();
}

void FileDlg::slotOkClicked()
{
    QList<KGetMetalink::Metaurl> metaurls = m_file->resources.metaurls;//TODO remove once metaurls are also shown
    QList<KGetMetalink::Pieces> pieces = m_file->verification.pieces;//TODO remove once the partial hashes are also shown
    m_file->clear();

    m_file->name = ui.name->text();
    m_file->size = ui.size->text().toLongLong();
    m_file->data.identity = uiData.identity->text();
    m_file->data.version = uiData.version->text();
    m_file->data.description = uiData.description->text();
    m_file->data.logo = KUrl(uiData.logo->text());
    if (!uiData.os->text().isEmpty()) {
        m_file->data.oses = uiData.os->text().split(i18nc("comma, to seperate members of a list", ","));
    }
    m_file->data.copyright = uiData.copyright->text();
    m_file->data.publisher.name = uiData.pub_name->text();
    m_file->data.publisher.url = KUrl(uiData.pub_url->text());
    m_file->data.languages << uiData.language->itemData(uiData.language->currentIndex()).toString();

    m_urlWidget->save();
    m_file->resources.metaurls = metaurls;


    //store the verification data
    for (int i = 0; i < m_verificationModel->rowCount(); ++i) {
        const QString type = m_verificationModel->index(i, VerificationModel::Type).data().toString();
        const QString hash = m_verificationModel->index(i, VerificationModel::Checksum).data().toString();
        m_file->verification.hashes[type] = hash;
    }
    m_file->verification.pieces = pieces;

    if (m_edit) {
        //the file has been edited
        emit fileEdited(m_initialFileName, m_file->name);
    } else {
        //a new file should be added, not edited
        emit addFile();
    }
}

#include "filedlg.moc"
