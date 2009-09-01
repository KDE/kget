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

#include "mirrormodel.h"

#include <QtCore/QAbstractListModel>
#include <QtGui/QSortFilterProxyModel>
#include <QtGui/QSpinBox>

#include <KComboBox>
#include <KGlobal>
#include <KLineEdit>
#include <KLocale>
#include <KStandardDirs>

MirrorDelegate::MirrorDelegate(QObject *parent)
  : QStyledItemDelegate(parent),
    m_countrySort(0)
{
}

MirrorDelegate::MirrorDelegate(QSortFilterProxyModel *countrySort, QObject *parent)
  : QStyledItemDelegate(parent),
    m_countrySort(countrySort)
{

}

QWidget *MirrorDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)

    if (index.isValid())
    {
        if (index.column() == MirrorItem::Url)
        {
            KLineEdit *line = new KLineEdit(parent);

            return line;
        }
        else if (index.column() == MirrorItem::Connections)
        {
            QSpinBox *numConnections = new QSpinBox(parent);
            numConnections->setRange(0, 20);

            return numConnections;
        }
        else if (index.column() == MirrorItem::Preference)
        {
            QSpinBox *preference = new QSpinBox(parent);
            preference->setRange(0, 100);

            return preference;
        }
        else if (index.column() == MirrorItem::Country)
        {
            if (m_countrySort)
            {
                KComboBox *countrySort = new KComboBox(parent);
                countrySort->setModel(m_countrySort);

                return countrySort;
            }
        }
    }

    return 0;
}

void MirrorDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (index.isValid() && editor)
    {
        if (index.column() == MirrorItem::Url)
        {
            KLineEdit *line = static_cast<KLineEdit*>(editor);
            const KUrl url = index.model()->data(index, Qt::EditRole).toUrl();
            line->setUrl(url);
        }
        else if (index.column() == MirrorItem::Connections)
        {
            QSpinBox *numConnections = static_cast<QSpinBox*>(editor);
            const int num = index.model()->data(index, Qt::EditRole).toInt();
            numConnections->setValue(num);
        }
        else if (index.column() == MirrorItem::Preference)
        {
            QSpinBox *preference = static_cast<QSpinBox*>(editor);
            const int num = index.model()->data(index, Qt::EditRole).toInt();
            preference->setValue(num);
        }
        else if (index.column() == MirrorItem::Country)
        {
            KComboBox *countrySort = static_cast<KComboBox*>(editor);
            const QString countryCode = index.model()->data(index, Qt::EditRole).toString();
            const int indexCountrySort = countrySort->findData(countryCode);
            countrySort->setCurrentIndex(indexCountrySort);
        }
    }
}

void MirrorDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if (index.isValid() && editor && model)
    {
        if (index.column() == MirrorItem::Url)
        {
            KLineEdit *line = static_cast<KLineEdit*>(editor);
            if (!line->text().isEmpty())
            {
                model->setData(index, line->text());
            }
        }
        else if (index.column() == MirrorItem::Connections)
        {
            QSpinBox *numConnections = static_cast<QSpinBox*>(editor);
            model->setData(index, numConnections->value());
        }
        else if (index.column() == MirrorItem::Preference)
        {
            QSpinBox *preference = static_cast<QSpinBox*>(editor);
            model->setData(index, preference->value());
        }
        else if (index.column() == MirrorItem::Country)
        {
            KComboBox *countrySort = static_cast<KComboBox*>(editor);
            const QString countryCode = countrySort->itemData(countrySort->currentIndex()).toString();
            model->setData(index, countryCode);
        }
    }
}

void MirrorDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index)
    editor->setGeometry(option.rect);
}

QSize MirrorDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    //make the sizeHint a little bit nicer to have more beautiful editors
    QSize hint;
    hint.setWidth(QStyledItemDelegate::sizeHint(option, index).width());
    hint.setHeight(option.fontMetrics.height() + 7);
    return hint;
}


MirrorItem::MirrorItem()
  : m_checked(Qt::Unchecked),
    m_numConnections(0)
{
}

QVariant MirrorItem::data(int column, int role) const
{
    if (column == MirrorItem::Used)
    {
        if (role == Qt::CheckStateRole)
        {
            return m_checked;
        }
    }
    else if (column == MirrorItem::Url)
    {
        if (role == Qt::DisplayRole)
        {
            return m_url.pathOrUrl();
        }
        else if ((role == Qt::UserRole) || (role == Qt::EditRole))
        {
            return QVariant(m_url);
        }
    }
    else if (column == MirrorItem::Connections)
    {
        if (role == Qt::DisplayRole)
        {
            if (m_numConnections)
            {
                return m_numConnections;
            }
            else
            {
                return i18n("not specified");
            }
        }
        else if ((role == Qt::EditRole) || (role == Qt::UserRole))
        {
            return m_numConnections;
        }
    }
    else if (column == MirrorItem::Preference)
    {
        if (role == Qt::DisplayRole)
        {
            if (m_preference)
            {
                return m_preference;
            }
            else
            {
                return i18n("not specified");
            }
        }
        else if ((role == Qt::EditRole) || (role == Qt::UserRole))
        {
            return m_preference;
        }
    }
    else if (column == MirrorItem::Country)
    {
        if (role == Qt::DisplayRole)
        {
            return m_countryName;
        }
        else if (role == Qt::DecorationRole)
        {
            return m_countryFlag;
        }
        else if ((role == Qt::UserRole) || (role == Qt::EditRole))
        {
            return m_countryCode;
        }
    }

    return QVariant();
}

Qt::ItemFlags MirrorItem::flags(int column) const
{
    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    if (column == MirrorItem::Used)
    {
        flags |= Qt::ItemIsUserCheckable;
    }
    else if (column == MirrorItem::Url)
    {
        flags |= Qt::ItemIsEditable;
    }
    else if (column == MirrorItem::Connections)
    {
        flags |= Qt::ItemIsEditable;
    }
    else if (column == MirrorItem::Preference)
    {
        flags |= Qt::ItemIsEditable;
    }
    else if (column == MirrorItem::Country)
    {
        flags |= Qt::ItemIsEditable;
    }

    return flags;
}

bool MirrorItem::setData(int column, const QVariant &value, int role)
{
    if ((column == MirrorItem::Used) && (role == Qt::CheckStateRole))
    {
        m_checked = static_cast<Qt::CheckState>(value.toInt());
        return true;
    }
    else if ((column == MirrorItem::Url) && (role == Qt::EditRole))
    {
        KUrl url;
        if (value.type() == QVariant::Url)
        {
            url = KUrl(value.toUrl());
        }
        else if (value.type() == QVariant::String)
        {
            url = KUrl(value.toString());
        }

        if (!url.isEmpty())
        {
            m_url = url;
            return true;
        }
    }
    else if ((column == MirrorItem::Connections) && (role == Qt::EditRole))
    {
        m_numConnections = value.toInt();
        return true;
    }
    else if ((column == MirrorItem::Preference) && (role == Qt::EditRole))
    {
        m_preference = value.toInt();
        return true;
    }
    else if ((column == MirrorItem::Country) && (role == Qt::EditRole))
    {
        m_countryCode = value.toString();
        m_countryName = KGlobal::locale()->countryCodeToName(m_countryCode);

        if (!m_countryName.isEmpty())
        {
            QString path = KStandardDirs::locate("locale", QString::fromLatin1("l10n/%1/flag.png").arg(m_countryCode));
            if (path.isEmpty())
            {
                m_countryFlag = KIcon();
            }
            else
            {
                m_countryFlag = KIcon(path);
            }
        }
        else
        {
            m_countryFlag = KIcon();
        }
        return true;
    }

    return false;
}


MirrorModel::MirrorModel(QObject *parent)
: QAbstractTableModel(parent)
{
}

MirrorModel::~MirrorModel()
{
    qDeleteAll(m_data);
}

int MirrorModel::rowCount(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return m_data.count();
    }
    else
    {
        return 0;
    }
}

int MirrorModel::columnCount(const QModelIndex &index) const
{
    if (index.isValid())
    {
        return 0;
    }

    return 5;
}

QVariant MirrorModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ((role != Qt::DisplayRole) || (orientation == Qt::Vertical))
    {
        return QVariant();
    }

    switch (section)
    {
        case MirrorItem::Url:
            return i18nc("Mirror as in server, in url", "Mirror");
        case MirrorItem::Connections:
            return i18nc("Number of paralell connections to the mirror", "Connections");
        case MirrorItem::Preference:
            return i18nc("The preference of the mirror", "Preference");
        case MirrorItem::Country:
            return i18nc("Location = country", "Location");
        default:
            return QVariant();
    }
}

QVariant MirrorModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || (index.row() >= rowCount()))
    {
        return QVariant();
    }

    return m_data.at(index.row())->data(index.column(), role);
}

Qt::ItemFlags MirrorModel::flags(const QModelIndex &index) const
{
    if (!index.isValid() && (index.row() >= rowCount()))
    {
        return 0;
    }

    return m_data.at(index.row())->flags(index.column());
}

bool MirrorModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() && (index.row() >= rowCount()))
    {
        return false;
    }

    const bool changed = m_data.at(index.row())->setData(index.column(), value, role);
    if (changed)
    {
        emit dataChanged(index, index);
    }
    return changed;
}

bool MirrorModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (parent.isValid() || (row < 0) || (count < 1) || (row + count > rowCount()))
    {
        return false;
    }

    beginRemoveRows(parent, row, row + count - 1);
    while (count)
    {
        MirrorItem *item = m_data[row];
        m_data.removeAt(row);
        delete item;
        --count;
    }
    endRemoveRows();

    return true;
}

void MirrorModel::addMirror(const KUrl &url, int numConnections, int preference, const QString &countryCode)
{
    if (!url.isValid())
    {
        return;
    }

    for (int i = 0; i < rowCount(); ++i)
    {
        //exists already, so remove the row
        if (KUrl(m_data.at(i)->data(MirrorItem::Url).toString()) == url)
        {
            removeRow(i);
            break;
        }
    }

    int index = rowCount();
    emit beginInsertRows(QModelIndex(), index, index);

    MirrorItem *item = new MirrorItem;
    m_data.append(item);
    item->setData(MirrorItem::Used, Qt::Checked, Qt::CheckStateRole);//every newly added mirror is set to checked automatically
    item->setData(MirrorItem::Url, QVariant(url));
    item->setData(MirrorItem::Connections, numConnections);
    item->setData(MirrorItem::Preference, preference);
    item->setData(MirrorItem::Country, countryCode);

    emit endInsertRows();
}

void MirrorModel::setMirrors(const QHash<KUrl, QPair<bool, int> > &mirrors)
{
    removeRows(0, rowCount());

    QHash<KUrl, QPair<bool, int> >::const_iterator it;
    QHash<KUrl, QPair<bool, int> >::const_iterator itEnd = mirrors.constEnd();
    for (it = mirrors.constBegin(); it != itEnd; ++it)
    {
        MirrorItem *item = new MirrorItem;
        item->setData(MirrorItem::Url, QVariant(it.key()));
        Qt::CheckState state = (*it).first ? Qt::Checked : Qt::Unchecked;
        item->setData(MirrorItem::Used, state, Qt::CheckStateRole);
        item->setData(MirrorItem::Connections, (*it).second);
        m_data.append(item);
    }

    emit reset();
}

QHash<KUrl, QPair<bool, int> > MirrorModel::availableMirrors() const
{
    QHash<KUrl, QPair<bool, int> > mirrors;
    foreach (MirrorItem *item, m_data)
    {
        bool used = (item->data(MirrorItem::Used, Qt::CheckStateRole).toInt() == Qt::Checked) ? true : false;
        const KUrl url = KUrl(item->data(MirrorItem::Url).toString());
        mirrors[url] = QPair<bool, int>(used, item->data(MirrorItem::Connections, Qt::UserRole).toInt());
    }
    return mirrors;
}

#include "mirrormodel.moc"
