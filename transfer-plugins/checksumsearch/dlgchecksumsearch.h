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

#ifndef DLGCHECKSUMSEARCH_H
#define DLGCHECKSUMSEARCH_H

#include "ui_checksumsearch.h"
#include "ui_checksumsearchadddlg.h"

#include "checksumsearchtransferdatasource.h"

#include <QtGui/QStyledItemDelegate>

#include <KCModule>

class QSortFilterProxyModel;
class QStandardItemModel;
class QStringListModel;

class ChecksumSearchAddDlg : public KDialog
{
    Q_OBJECT

    public:
        ChecksumSearchAddDlg(QStringListModel *modesModel, QStringListModel *typesModel, QWidget *parent = 0, Qt::WFlags flags = 0);

    signals:
        /**
        * Emitted when the dialog gets accepted
        * @param change the string that should change the source url by mode
        * @param mode the change mode
        * @param type the checksum type, can be an empty string
        */
        void addItem(const QString &change, int mode, const QString &type);

    private slots:
        /**
        * Enables or disables buttons depending on if the user entered text or not, also changes
        * the label etc.
        */
        void slotUpdate();

        void slotAccpeted();

    private:
        Ui::ChecksumSearchAddDlg ui;

        QStringListModel *m_modesModel;
        QStringListModel *m_typesModel;

        static const KUrl URL;
};

class ChecksumDelegate : public QStyledItemDelegate
{
    Q_OBJECT

    public:
        ChecksumDelegate(QObject *parent = 0);
        ChecksumDelegate(QStringListModel *modesModel, QStringListModel *typesModel, QObject *parent = 0);

        QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
        void setEditorData(QWidget *editor, const QModelIndex &index) const;
        void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
        void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    private:
        QStringListModel *m_modesModel;
        QStringListModel *m_typesModel;
};

class DlgChecksumSettingsWidget : public KCModule
{
    Q_OBJECT

    public:
        explicit DlgChecksumSettingsWidget(QWidget *parent = 0, const QVariantList &args = QVariantList());
        ~DlgChecksumSettingsWidget();

    public slots:
        void save();
        void load();

    private slots:
        /**
         * Oppens the AddDlg
         */
        void slotAdd();

        /**
         * Remove the selected indexes
         */
        void slotRemove();

        /**
         * Enables or disables buttons depending on if the user entered text or not, also changes
         * the label etc.
         */
        void slotUpdate();

        /**
         * Adds a new item defining how to proceed a search for checksums to the model
         * @param change the string that should change the source url by mode
         * @param mode the change mode, defined in verifier.h, using int instead of enum as convenience
         * @param type the checksum type, like e.g. "md5", empty if you do not know that
         * e.g. if change is "CHECKSUMS" you cannot know which checksums are present
         */
        void slotAddItem(const QString &change, int mode, const QString &type = QString());

    private:
        Ui::ChecksumSearch ui;
        KDialog *m_parent;
        QStandardItemModel *m_model;
        QSortFilterProxyModel *m_proxy;
        QStringList m_modes;
        QStringListModel *m_modesModel;
        QStringListModel *m_typesModel;
};

#endif // DLGCHECKSUMSEARCH_H
