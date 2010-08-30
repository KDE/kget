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

#ifndef AUTOPASTEMODEL
#define AUTOPASTEMODEL

#include <QtCore/QAbstractTableModel>
#include <QtGui/QStyledItemDelegate>

class AutoPasteDelegate : public QStyledItemDelegate
{
    Q_OBJECT

    public:
        AutoPasteDelegate(QAbstractItemModel *types, QAbstractItemModel *syntaxes, QObject *parent = 0);

        QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
        void setEditorData(QWidget *editor, const QModelIndex &index) const;
        void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
        void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
        QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

    private:
        QAbstractItemModel *m_types;
        QAbstractItemModel *m_syntaxes;
};

class AutoPasteModel : public QAbstractTableModel
{
    Q_OBJECT

    public:
        enum DataType {
            Type = 0,
            Pattern,
            PatternSyntax
        };
        enum TypeData {
            Include = 0,
            Exclude
        };
        enum PatternSyntaxData {
            Wildcard = 0,
            RegExp
        };

        explicit AutoPasteModel(QObject *parent = 0);
        ~AutoPasteModel();

        int rowCount(const QModelIndex &index = QModelIndex()) const;
        int columnCount(const QModelIndex &index = QModelIndex()) const;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
        Qt::ItemFlags flags(const QModelIndex &index) const;
        bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
        bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());

        /**
         * Adds an an item
         * @note do not use this for loading data
         * @see load()
         */
        void addItem(TypeData dataType, PatternSyntaxData patternSyntax, const QString &pattern);

        /**
         * Moves an item to a new position
         * @param sourceRow the current row of the selected item
         * @param destinationRow is the new row before the move,
         * i.e. if sourceRow was not removed from the model
         * @see QAbstractItemModel::beginMoveRows()
         */
        bool moveItem(int sourceRow, int destinationRow);

    public slots:
        /**
         * Loads the stored data
         * @see save()
         */
        void load();

        /**
         * Saves the current data
         * @see load()
         */
        void save();

        /**
         * Resets the model to the default data
         */
        void resetDefaults();

    private:
        void addItems(const QList<int> &dataTypes, const QList<int> patternSyntaxes, const QStringList &patterns);

    private:
        struct Data
        {
            TypeData type;
            QString pattern;
            PatternSyntaxData syntax;
        };

        QList<Data> m_data;
};

#endif

