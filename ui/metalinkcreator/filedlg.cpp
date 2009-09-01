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

#include "../mirror/mirrormodel.h"

#include <QtGui/QSortFilterProxyModel>

#include <KLocale>

FileDlg::FileDlg(KGetMetalink::File *file, const QStringList &currentFileNames, QSortFilterProxyModel *countrySort, QSortFilterProxyModel *languageSort, QWidget *parent, bool edit)
  : KDialog(parent),
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

    ui.name->setText(m_file->name);
    ui.identity->setText(m_file->data.identity);
    ui.version->setText(m_file->data.version);
    ui.description->setText(m_file->data.description);
    ui.logo->setUrl(m_file->data.logo);
    ui.os->setText(m_file->data.os);
    ui.copyright->setText(m_file->data.copyright);
    ui.pub_name->setText(m_file->data.publisher.name);
    ui.pub_url->setUrl(m_file->data.publisher.url);
    ui.lic_name->setText(m_file->data.license.name);
    ui.lic_url->setUrl(m_file->data.license.url);

    if (m_file->size)
    {
        ui.size->setText(QString::number(m_file->size));
    }


    //create the language selection
    ui.language->setModel(languageSort);
    const int index = ui.language->findData(m_file->data.language);
    ui.language->setCurrentIndex(index);


    //create the verification stuff
    m_verificationModel = new VerificationModel(this);
    QHash<QString, QString>::const_iterator it;
    QHash<QString, QString>::const_iterator itEnd = m_file->verification.hashes.constEnd();
    for (it = m_file->verification.hashes.constBegin(); it != itEnd; ++it)
    {
        m_verificationModel->addChecksum(it.key(), it.value());
    }
    ui.add_hash->setGuiItem(KStandardGuiItem::add());
    ui.remove_hash->setGuiItem(KStandardGuiItem::remove());
    ui.used_hashes->setModel(m_verificationModel);
    slotUpdateVerificationButtons();

    connect(m_verificationModel, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(slotUpdateVerificationButtons()));
    connect(ui.used_hashes, SIGNAL(clicked(const QModelIndex&)), this, SLOT(slotUpdateVerificationButtons()));
    connect(ui.add_hash, SIGNAL(pressed()), this, SLOT(slotAddHash()));
    connect(ui.remove_hash, SIGNAL(pressed()), this, SLOT(slotRemoveHash()));


    slotUpdateOkButton();

    connect(ui.name, SIGNAL(textEdited(QString)), this, SLOT(slotUpdateOkButton()));
    connect(this, SIGNAL(okClicked()), this, SLOT(slotOkClicked()));

    setCaption(i18n("File Properties"));
}

void FileDlg::slotUpdateOkButton()
{
    bool hasName = !ui.name->text().isEmpty();
    bool hasUrls =  m_urlWidget->hasUrls();
    bool isDuplicate = (m_currentFileNames.indexOf(ui.name->text()) > -1);

    const QString enterName = i18n("Enter a filename.");
    const QString enterUrl = i18n("Enter at least one url.");
    const QString duplicate = i18n("The filename exists already, choose a different one.");

    QString text;

    if (isDuplicate)
    {
        text = duplicate;
        if (!hasUrls)
        {
            text += ' ' + enterUrl;
        }
    }
    else
    {
        //no name yet
        if (!hasName)
        {
            text = enterName;
        }
        //no urls yet
        if (!hasUrls)
        {
            if (!text.isEmpty())
            {
                text += ' ';
            }
            text += enterUrl;
        }
        //all requirements fulfilled
        if (text.isEmpty())
        {
            text = i18n("Required data entered, also consider to enter additional information.");
        }
    }

    ui.ktitlewidget->setText(text, KTitleWidget::InfoMessage);

    enableButtonOk(hasName && hasUrls && !isDuplicate);
}

void FileDlg::slotUpdateVerificationButtons()
{
    ui.remove_hash->setEnabled(ui.used_hashes->selectionModel()->selectedIndexes().count());
}

void FileDlg::slotRemoveHash()
{
    const QModelIndexList indexes = ui.used_hashes->selectionModel()->selectedRows();
    foreach (const QModelIndex &index, indexes)
    {
        m_verificationModel->removeRow(index.row());
    }
    slotUpdateVerificationButtons();
}

void FileDlg::slotAddHash()
{
    VerificationAddDlg *dialog = new VerificationAddDlg(m_verificationModel, this);
    dialog->show();
}

void FileDlg::slotOkClicked()
{//TODO metaurl!!!
    m_file->clear();

    m_file->name = ui.name->text();
    m_file->size = ui.size->text().toLongLong();
    m_file->data.identity = ui.identity->text();
    m_file->data.version = ui.version->text();
    m_file->data.description = ui.description->text();
    m_file->data.logo = KUrl(ui.logo->text());
    m_file->data.os = ui.os->text();
    m_file->data.copyright = ui.copyright->text();
    m_file->data.publisher.name = ui.pub_name->text();
    m_file->data.publisher.url = KUrl(ui.pub_url->text());
    m_file->data.license.name = ui.lic_name->text();
    m_file->data.license.url = KUrl(ui.lic_url->text());

    m_file->data.language = ui.language->itemData(ui.language->currentIndex()).toString();

    m_urlWidget->save();


    //store the verification data
    for (int i = 0; i < m_verificationModel->rowCount(); ++i)
    {
        const QString type = m_verificationModel->index(i, VerificationModel::Type).data().toString();
        const QString hash = m_verificationModel->index(i, VerificationModel::Checksum).data().toString();
        m_file->verification.hashes[type] = hash;
    }

    //the file has been edited
    if (m_edit)
    {
        emit fileEdited(m_initialFileName, m_file->name);
    }
    //a new file should be added, not edited
    else
    {
        emit addFile();
    }
}

#include "filedlg.moc"
