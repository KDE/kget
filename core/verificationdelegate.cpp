/**************************************************************************
*   Copyright (C) 2009-2011 Matthias Fuchs <mat69@gmx.net>                *
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

#include "verificationdelegate.h"
#include "verifier.h"
#include "verificationmodel.h"

#include <KComboBox>
#include <KLineEdit>

struct VerificationDelegatePrivate
{
    VerificationDelegatePrivate()
    {
    }

    ~VerificationDelegatePrivate()
    {
    }

    QStringList hashTypes;
};

VerificationDelegate::VerificationDelegate(QObject *parent)
  : QStyledItemDelegate(parent),
    d(new VerificationDelegatePrivate)
{
    d->hashTypes = Verifier::supportedVerficationTypes();
    d->hashTypes.sort();
}

QWidget *VerificationDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)

    if (index.isValid()) {
        if (index.column() == VerificationModel::Type) {
            if (d->hashTypes.count()) {
                KComboBox *hashTypes = new KComboBox(parent);
                hashTypes->addItems(d->hashTypes);

                return hashTypes;
            }
        } else if (index.column() == VerificationModel::Checksum) {
            return new KLineEdit(parent);
        }
    }

    return 0;
}

void VerificationDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (index.isValid() && editor) {
        if (index.column() == VerificationModel::Type) {
            KComboBox *hashTypes = static_cast<KComboBox*>(editor);
            const QString hashType = index.data().toString();
            hashTypes->setCurrentItem(hashType);
        } else if (index.column() == VerificationModel::Checksum) {
            KLineEdit *line = static_cast<KLineEdit*>(editor);
            const QString checksum = index.data().toString();
            line->setText(checksum);
        }
    }
}

void VerificationDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if (index.isValid() && editor && model) {
        if (index.column() == VerificationModel::Type) {
            KComboBox *hashTypes = static_cast<KComboBox*>(editor);
            model->setData(index, hashTypes->currentText());
        } else if (index.column() == VerificationModel::Checksum) {
            KLineEdit *line = static_cast<KLineEdit*>(editor);
            model->setData(index, line->text());
        }
    }
}

void VerificationDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index)
    editor->setGeometry(option.rect);
}

QSize VerificationDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    //make the sizeHint a little bit nicer to have more beautiful editors
    QSize hint;
    hint.setWidth(QStyledItemDelegate::sizeHint(option, index).width());
    hint.setHeight(option.fontMetrics.height() + 7);
    return hint;
}
