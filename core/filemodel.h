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

#ifndef FILEMODEL_H
#define FILEMODEL_H

#include "kget_export.h"
#include "transfer.h"

#include <QUrl>

#include <QAbstractItemModel>
#include <QList>
#include <QVariant>

#include <QIcon>
#include <kio/global.h>

class FileModel;

class KGET_EXPORT FileItem
{
public:
    explicit FileItem(const QString &name, FileItem *parent = nullptr);
    ~FileItem();

    enum DataType {
        File = 0,
        Status,
        Size,
        ChecksumVerified,
        SignatureVerified
    };

    void appendChild(FileItem *child);

    FileItem *child(int row);
    int childCount() const;
    /**
     * Returns true if the index represents a file
     */
    bool isFile() const;
    int columnCount() const;
    QVariant data(int column, int role) const;
    bool setData(int column, const QVariant &value, FileModel *model, int role = Qt::EditRole);
    int row() const;
    FileItem *parent();

private:
    /**
     * Add the totalsize to the parent
     */
    void addSize(KIO::fileoffset_t size, FileModel *model);

    /**
     * If all siblings of this have the same state the parent of item will also be set
     * to that state
     */
    void checkParents(Qt::CheckState state, FileModel *model);

    /**
     * All children of item will be set to state
     */
    void checkChildren(Qt::CheckState state, FileModel *model);

private:
    QList<FileItem *> m_childItems;
    mutable QIcon m_mimeType;
    QString m_name;
    Qt::CheckState m_state;
    Job::Status m_status;
    KIO::fileoffset_t m_totalSize;
    int m_checkusmVerified;
    int m_signatureVerified;
    FileItem *m_parent;
};

/**
 * This model represents the files that are being downloaded
 * @note whenever a method takes a url as argument use the url to the file
 * destination on your hard disk, the file does not need to exist though
 */

class KGET_EXPORT FileModel : public QAbstractItemModel
{
    Q_OBJECT

    friend class FileItem;

public:
    FileModel(const QList<QUrl> &files, const QUrl &destDirectory, QObject *parent = nullptr);
    ~FileModel() override;

    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex index(const QUrl &file, int column);
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    void rename(const QModelIndex &file, const QString &newName);

    /**
     * Returns a list of pointers to all files of this model
     * @note it would be possible to directly interact with the model data that way,
     * though that is discouraged
     */
    QModelIndexList fileIndexes(int column) const;

    /**
     * The url on the filesystem (no check if the file exists yet!) of index,
     * it can be a folder or file
     */
    QUrl getUrl(const QModelIndex &index);

    /**
     * Set the url to the directory the files are stored in, the filemodel stores
     * its entries as relative path to the base directory
     * @param newDirectory the base directory for the model
     */
    void setDirectory(const QUrl &newDirectory);

    /**
     * Set a custom status text for status
     * @note QString() removes the custom text for status
     */
    void setCustomStatusText(Job::Status status, const QString &text);

    /**
     * Set a custom status icon for status
     * @note QIcon() removes the custom icon for status
     */
    void setCustomStatusIcon(Job::Status status, const QIcon &icon);

    /**
     * Checks if the download for file has been finished
     */
    bool downloadFinished(const QUrl &file);

    /**
     * Returns true if the index represents a file
     */
    bool isFile(const QModelIndex &index) const;

public Q_SLOTS:
    /**
     * Watches if the check state changes, the result of that will be emitted
     * when stopWatchCheckState() is being called()
     */
    void watchCheckState();

    /**
     * Emits checkStateChanged if a CheckState of an entry changed
     */
    void stopWatchCheckState();

Q_SIGNALS:
    void rename(const QUrl &oldUrl, const QUrl &newUrl);
    void checkStateChanged();
    void fileFinished(const QUrl &file);

private:
    void setupModelData(const QList<QUrl> &files);

    /**
     * The url on the filesystem (no check if the file exists yet!) of item,
     * it can be a folder or file
     */
    QUrl getUrl(FileItem *item);

    /**
     * Get the path of item
     * @note path means here the part between m_destDirectory and the filename
     * e.g. m_destDirectory = "file:///home/user/Downloads", path = "Great Album/",
     * filename = "Song 1.ogg"
     */
    QString getPath(FileItem *item);

    /**
     * Get the item of the corresponding url
     */
    FileItem *getItem(const QUrl &file);

    void changeData(int row, int column, FileItem *item, bool fileFinished = false);

private Q_SLOTS:
    void renameFailed(const QUrl &beforeRename, const QUrl &afterRename);

private:
    FileItem *m_rootItem;
    QUrl m_destDirectory;
    QList<QVariant> m_header;
    bool m_checkStateChanged;

    QHash<QUrl, FileItem *> m_itemCache; // used to make getItem faster

    /**
     * Stores links to all files
     */
    QList<FileItem *> m_files;

    QHash<Job::Status, QString> m_customStatusTexts;
    QHash<Job::Status, QIcon> m_customStatusIcons;
};

#endif
