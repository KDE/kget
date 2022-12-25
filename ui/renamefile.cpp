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

#include <QFile>
#include <QPushButton>

#include <KLocalizedString>

RenameFile::RenameFile(FileModel *model, const QModelIndex &index, QWidget *parent, Qt::WindowFlags flags)
    : QDialog(parent, flags)
    , m_model(model)
    , m_index(index)
{
    setWindowTitle(i18n("Rename File"));
    ui.setupUi(this);

    const QString originalName = m_model->data(m_index, Qt::DisplayRole).toString();
    m_dest = m_model->getUrl(m_index).adjusted(QUrl::RemoveFilename);

    ui.label->setText(i18n("Rename %1 to:", originalName));
    ui.name->setText(originalName);

    ui.buttonBox->button(QDialogButtonBox::Ok)->setText(i18n("&Rename"));
    ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    // ui.buttonBox->button(QDialogButtonBox::Ok)->setShortcut(Qt::CTRL | Qt::Key_Return);

    connect(ui.name, &KLineEdit::textEdited, this, &RenameFile::updateButton);
    connect(ui.buttonBox, &QDialogButtonBox::accepted, this, &RenameFile::accept);
    connect(ui.buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(this, &QDialog::accepted, this, &RenameFile::rename);
}

void RenameFile::updateButton()
{
    const QString newName = ui.name->text();
    QUrl dest = m_dest;
    dest.setPath(m_dest.path() + newName);

    const bool enabled = !newName.isEmpty() && !QFile::exists(dest.toString());
    ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(enabled);
}

void RenameFile::rename()
{
    const QString newName = ui.name->text();
    m_model->rename(m_index, newName);
}
