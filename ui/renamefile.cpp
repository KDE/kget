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

#include "renamefile.h"
#include "core/filemodel.h"

#include <QtCore/QFile>

#include <KLocale>

RenameFile::RenameFile(FileModel *model, const QModelIndex &index, QWidget *parent, Qt::WFlags flags)
  : KDialog(parent, flags),
    m_model(model),
    m_index(index)
{
    setCaption(i18n("Rename File"));
    showButtonSeparator(true);
    QWidget *widget = new QWidget(this);
    ui.setupUi(widget);
    setMainWidget(widget);

    const QString originalName = m_model->data(m_index, Qt::DisplayRole).toString();
    m_dest = m_model->getUrl(m_index).upUrl();

    ui.label->setText(i18n("Rename %1 to:", originalName));
    ui.name->setText(originalName);

    setButtonText(KDialog::Ok, i18n("&Rename"));
    enableButtonOk(false);

    connect(ui.name, SIGNAL(textEdited(QString)), this, SLOT(updateButton()));
    connect(this, SIGNAL(okClicked()), this, SLOT(rename()));
}

void RenameFile::updateButton()
{
    const QString newName = ui.name->text();
    KUrl dest = m_dest;
    dest.addPath(newName);

    const bool enabled = !newName.isEmpty() && !QFile::exists(dest.pathOrUrl());
    enableButtonOk(enabled);
}

void RenameFile::rename()
{
    const QString newName = ui.name->text();
    m_model->rename(m_index, newName);
}

#include "renamefile.moc"
