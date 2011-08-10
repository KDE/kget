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

#include "dragdlg.h"
#include "metalinker.h"
#include "urlwidget.h"

#include "core/verifier.h"

#include <QtGui/QCheckBox>
#include <QtGui/QSortFilterProxyModel>

DragDlg::DragDlg(KGetMetalink::Resources *resources, KGetMetalink::CommonData *commonData, QSortFilterProxyModel *countrySort, QSortFilterProxyModel *languageSort, QWidget *parent)
  : KGetSaveSizeDialog("DragDlg", parent),
    m_resources(resources),
    m_commonData(commonData)
{
    QWidget *widget = new QWidget(this);
    ui.setupUi(widget);
    setMainWidget(widget);

    m_urlWidget = new UrlWidget(this);
    m_urlWidget->init(m_resources, countrySort);
    ui.urlLayout->addWidget(m_urlWidget->widget());

    QWidget *data = new QWidget(this);
    uiData.setupUi(data);
    ui.dataLayout->addWidget(data);

    QVBoxLayout *layout = new QVBoxLayout;
    QStringList verifierTypes = Verifier::supportedVerficationTypes();
    verifierTypes.sort();

    //NOTE only supports the types that are supported by the Metalink 4.0 specification -- "Hash Function Textual Names"
    foreach (const QString &type, verifierTypes) {
        if (type.contains("sha", Qt::CaseInsensitive) || type.contains("md5", Qt::CaseInsensitive)) {
            QCheckBox *checkBox = new QCheckBox(type, this);
            layout->addWidget(checkBox);
            m_checkBoxes.append(checkBox);
        }
    }

    ui.groupBox->setLayout(layout);

    //create the language selection
    uiData.language->setModel(languageSort);
    uiData.language->setCurrentIndex(-1);

    connect(this, SIGNAL(accepted()), this, SLOT(slotFinished()));

    setCaption(i18n("Import dropped files"));
}

void DragDlg::slotFinished()
{
    m_urlWidget->save();

    QStringList used;
    foreach (QCheckBox *checkbox, m_checkBoxes)
    {
        if (checkbox->isChecked())
        {
            used.append(checkbox->text().remove('&'));
        }
    }

    m_commonData->identity = uiData.identity->text();
    m_commonData->version = uiData.version->text();
    m_commonData->description = uiData.description->text();
    m_commonData->logo = KUrl(uiData.logo->text());
    if (uiData.os->text().isEmpty()) {
        m_commonData->oses.clear();
    } else {
        m_commonData->oses = uiData.os->text().split(i18nc("comma, to seperate members of a list", ","));
    }
    m_commonData->copyright = uiData.copyright->text();
    m_commonData->publisher.name = uiData.pub_name->text();
    m_commonData->publisher.url = KUrl(uiData.pub_url->text());
    m_commonData->languages << uiData.language->itemData(uiData.language->currentIndex()).toString();

    emit usedTypes(used, ui.partialChecksums->isChecked());
}

#include "dragdlg.moc"
