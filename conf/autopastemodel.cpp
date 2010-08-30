/***************************************************************************
*   Copyright (C) 2010 Matthias Fuchs <mat69@gmx.net>                     *
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

#include "autopastemodel.h"
#include "settings.h"

#include <KComboBox>
#include <KIcon>
#include <KLineEdit>
#include <KLocale>

AutoPasteDelegate::AutoPasteDelegate(QAbstractItemModel *types, QAbstractItemModel *syntaxes, QObject *parent)
  : QStyledItemDelegate(parent),
    m_types(types),
    m_syntaxes(syntaxes)
{
}

QWidget *AutoPasteDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)

    if (!index.isValid()) {
        return 0;
    }

    switch(index.column()) {
        case AutoPasteModel::Type: {
            KComboBox *types = new KComboBox(parent);
            types->setModel(m_types);
            return types;
        }
        case AutoPasteModel::Pattern: {
            KLineEdit *pattern = new KLineEdit(parent);
            return pattern;
        }
        case AutoPasteModel::PatternSyntax: {
            KComboBox *syntaxes = new KComboBox(parent);
            syntaxes->setModel(m_syntaxes);
            return syntaxes;
        }
        default:
            return 0;
    }
}

void AutoPasteDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (!index.isValid() || !editor) {
        return;
    }

    switch (index.column()) {
        case AutoPasteModel::Type: {
            KComboBox *type = static_cast<KComboBox*>(editor);
            const int row = type->findData(index.data(Qt::EditRole));
            type->setCurrentIndex(row);
            break;
        }
        case AutoPasteModel::Pattern: {
            KLineEdit *line = static_cast<KLineEdit*>(editor);
            line->setText(index.data(Qt::EditRole).toString());
            break;
        }
        case AutoPasteModel::PatternSyntax: {
            KComboBox *syntax = static_cast<KComboBox*>(editor);
            const int row = syntax->findData(index.data(Qt::EditRole));
            syntax->setCurrentIndex(row);
            break;
        }
        default:
            break;
    }
}

void AutoPasteDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if (!index.isValid() || !editor || !model) {
        return;
    }

    switch (index.column()) {
        case AutoPasteModel::Type: {
            KComboBox *typeBox = static_cast<KComboBox*>(editor);
            const int type = typeBox->itemData(typeBox->currentIndex()).toInt();
            model->setData(index, type);
            break;
        }
        case AutoPasteModel::Pattern: {
            KLineEdit *line = static_cast<KLineEdit*>(editor);
            const QString text = line->text();
            if (!text.isEmpty()) {
                model->setData(index, text);
            }
            break;
        }
        case AutoPasteModel::PatternSyntax: {
            KComboBox *syntaxBox = static_cast<KComboBox*>(editor);
            const int syntax = syntaxBox->itemData(syntaxBox->currentIndex()).toInt();
            model->setData(index, syntax);
            break;
        }
        default:
            break;
    }
}

void AutoPasteDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index)
    editor->setGeometry(option.rect);
}

QSize AutoPasteDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    //make the sizeHint a little bit nicer to have more beautiful editors
    QSize hint;
    hint.setWidth(QStyledItemDelegate::sizeHint(option, index).width());
    hint.setHeight(option.fontMetrics.height() + 7);
    return hint;
}

AutoPasteModel::AutoPasteModel(QObject *parent)
  : QAbstractTableModel(parent)
{
}

AutoPasteModel::~AutoPasteModel()
{
}

int AutoPasteModel::rowCount(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return m_data.count();
    }

    return 0;
}

int AutoPasteModel::columnCount(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return 3;
    }

    return 0;
}

QVariant AutoPasteModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Vertical) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        if (section == Pattern) {
            return i18n("Pattern");
        } else if (section == PatternSyntax) {
            return i18n("Syntax");
        }
    }

    return QVariant();
}

QVariant AutoPasteModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    const int column = index.column();
    const int row = index.row();

    switch (column) {
        case Type: {
            if (role == Qt::DecorationRole) {
                return (m_data[row].type == Include ? KIcon("list-add") : KIcon("list-remove"));
            } else if ((role == Qt::UserRole) || (role == Qt::EditRole)) {
                return m_data[row].type;
            }
            break;
        }
        case Pattern: {
            if ((role == Qt::DisplayRole) || (role == Qt::EditRole) || (role == Qt::UserRole)) {
                return m_data[row].pattern;
            }
            break;
        }
        case PatternSyntax: {
            if (role == Qt::DisplayRole) {
                return (m_data[row].syntax == Wildcard ? i18n("Escape sequences") : i18n("Regular expression"));
            } else if ((role == Qt::UserRole) || (role == Qt::EditRole)) {
                return m_data[row].syntax;
            }
            break;
        }
        default:
            return QVariant();
    }

    return QVariant();
}

Qt::ItemFlags AutoPasteModel::flags(const QModelIndex &index) const
{
    Q_UNUSED(index)

    return (Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
}

bool AutoPasteModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole) {
        return false;
    }

    const int row = index.row();
    const int column = index.column();

    switch (column) {
        case Type: {
            m_data[row].type = static_cast<TypeData>(value.toInt());
            break;
        }
        case Pattern: {
            m_data[row].pattern = value.toString();
            break;
        }
        case PatternSyntax: {
            m_data[row].syntax = static_cast<PatternSyntaxData>(value.toInt());
            break;
        }
        default:
            return false;
    }

    emit dataChanged(index, index);
    return true;
}

bool AutoPasteModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (parent.isValid() || (row < 0) || (count < 1) || (row + count > rowCount())) {
        return false;
    }

    beginRemoveRows(parent, row, row + count - 1);
    while (count) {
        m_data.removeAt(row);
        --count;
    }
    endRemoveRows();

    return true;
}

void AutoPasteModel::addItem(TypeData dataType, PatternSyntaxData patternSyntax, const QString &pattern)
{
    addItems(QList<int>() << dataType, QList<int>() << patternSyntax, QStringList() << pattern);
}

void AutoPasteModel::addItems(const QList<int> &dataTypes, const QList<int> patternSyntaxes, const QStringList &patterns)
{
    const int row = rowCount();
    const int numItems = patterns.count();
    beginInsertRows(QModelIndex(), row, row + numItems - 1);

    for (int i = 0; i < numItems; ++i) {
        Data data;
        data.type = static_cast<TypeData>(dataTypes[i]);
        data.pattern = patterns[i];
        data.syntax = static_cast<PatternSyntaxData>(patternSyntaxes[i]);
        m_data.append(data);
    }

    endInsertRows();
}

bool AutoPasteModel::moveItem(int sourceRow, int destinationRow)
{
    if (!beginMoveRows(QModelIndex(), sourceRow, sourceRow, QModelIndex(), destinationRow)) {
        return false;
    }

    //beginMoveRows asks for different data, than QList::move does, see the 4.7 docs
    if (sourceRow + 2 == destinationRow) {
        --destinationRow;
    }
    m_data.move(sourceRow, destinationRow);
    endMoveRows();

    return true;
}

void AutoPasteModel::load()
{
    //remove all old items if there are any
    if (rowCount()) {
        beginRemoveRows(QModelIndex(), 0, rowCount() - 1);
        m_data.clear();
        endRemoveRows();
    }
    addItems(Settings::autoPasteTypes(), Settings::autoPastePatternSyntaxes(), Settings::autoPastePatterns());
}

void AutoPasteModel::save()
{
    QList<int> types;
    QList<int> syntaxes;
    QStringList patterns;
    foreach (const Data &data, m_data) {
        types << data.type;
        syntaxes << data.syntax;
        patterns << data.pattern;
    }

    Settings::self()->setAutoPasteTypes(types);
    Settings::self()->setAutoPastePatternSyntaxes(syntaxes);
    Settings::self()->setAutoPastePatterns(patterns);
    Settings::self()->writeConfig();
}

void AutoPasteModel::resetDefaults()
{
    QStringList names = QStringList() << "AutoPastePatterns" << "AutoPasteTypes" << "AutoPastePatternSyntaxes";
    foreach (const QString &name, names) {
        KConfigSkeletonItem *item = Settings::self()->findItem(name);
        if (item) {
            item->readDefault(Settings::self()->config());
        }
    }

    load();
}

