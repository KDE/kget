/***************************************************************************
 *   Copyright (C) 2007 by Joris Guisson and Ivan Vasic                    *
 *   joris.guisson@gmail.com                                               *
 *   ivasic@gmail.com                                                      *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#include "torrentfiletreemodel.h"

#include <KLocalizedString>

#include <QIcon>
#include <QMimeDatabase>
#include <QMimeType>
#include <QSortFilterProxyModel>
#include <QTreeView>

#include <bcodec/bdecoder.h>
#include <bcodec/bencoder.h>
#include <bcodec/bnode.h>
#include <interfaces/torrentfileinterface.h>
#include <interfaces/torrentinterface.h>
#include <util/functions.h>
#include <util/log.h>

using namespace bt;

namespace kt
{

TorrentFileTreeModel::Node::Node(Node *parent, bt::TorrentFileInterface *file, const QString &name, const bt::Uint32 total_chunks)
    : parent(parent)
    , file(file)
    , name(name)
    , size(0)
    , chunks(total_chunks)
    , chunks_set(false)
    , percentage(0.0f)
{
    chunks.setAll(false);
}

TorrentFileTreeModel::Node::Node(Node *parent, const QString &name, const bt::Uint32 total_chunks)
    : parent(parent)
    , file(nullptr)
    , name(name)
    , size(0)
    , chunks(total_chunks)
    , chunks_set(false)
    , percentage(0.0f)
{
    chunks.setAll(false);
}

TorrentFileTreeModel::Node::~Node()
{
    qDeleteAll(children);
}

void TorrentFileTreeModel::Node::insert(const QString &path, bt::TorrentFileInterface *file, bt::Uint32 num_chunks)
{
    int p = path.indexOf(bt::DirSeparator());
    if (p == -1) {
        // the file is part of this directory
        children.append(new Node(this, file, path, num_chunks));
    } else {
        QString subdir = path.left(p);
        foreach (Node *n, children) {
            if (n->name == subdir) {
                n->insert(path.mid(p + 1), file, num_chunks);
                return;
            }
        }

        Node *n = new Node(this, subdir, num_chunks);
        children.append(n);
        n->insert(path.mid(p + 1), file, num_chunks);
    }
}

int TorrentFileTreeModel::Node::row()
{
    if (parent)
        return parent->children.indexOf(this);
    else
        return 0;
}

bt::Uint64 TorrentFileTreeModel::Node::fileSize(const bt::TorrentInterface *tc)
{
    if (size > 0)
        return size;

    if (!file) {
        // directory
        foreach (Node *n, children)
            size += n->fileSize(tc);
    } else {
        size = file->getSize();
    }
    return size;
}

void TorrentFileTreeModel::Node::fillChunks()
{
    if (chunks_set)
        return;

    if (!file) {
        foreach (Node *n, children) {
            n->fillChunks();
            chunks.orBitSet(n->chunks);
        }
    } else {
        for (Uint32 i = file->getFirstChunk(); i <= file->getLastChunk(); ++i)
            chunks.set(i, true);
    }
    chunks_set = true;
}

void TorrentFileTreeModel::Node::updatePercentage(const BitSet &havechunks)
{
    if (!chunks_set)
        fillChunks(); // make sure we know the chunks which are part of this node

    if (file) {
        percentage = file->getDownloadPercentage();
    } else {
        if (havechunks.numOnBits() == 0 || chunks.numOnBits() == 0) {
            percentage = 0.0f;
        } else if (havechunks.allOn()) {
            percentage = 100.0f;
        } else {
            // take the chunks of the node and
            // logical and them with the chunks we have
            BitSet tmp(chunks);
            tmp.andBitSet(havechunks);

            percentage = 100.0f * ((float)tmp.numOnBits() / (float)chunks.numOnBits());
        }
    }

    if (parent)
        parent->updatePercentage(havechunks); // update the percentage of the parent
}

void TorrentFileTreeModel::Node::initPercentage(const bt::TorrentInterface *tc, const bt::BitSet &havechunks)
{
    if (!chunks_set)
        fillChunks();

    if (!tc->getStats().multi_file_torrent) {
        percentage = bt::Percentage(tc->getStats());
        return;
    }

    if (file) {
        percentage = file->getDownloadPercentage();
    } else {
        if (havechunks.numOnBits() == 0 || chunks.numOnBits() == 0) {
            percentage = 0.0f;
        } else if (havechunks.allOn()) {
            percentage = 100.0f;
        } else {
            // take the chunks of the node and
            // logical and them with the chunks we have
            BitSet tmp(chunks);
            tmp.andBitSet(havechunks);

            percentage = 100.0f * ((float)tmp.numOnBits() / (float)chunks.numOnBits());
        }

        foreach (Node *n, children)
            n->initPercentage(tc, havechunks); // update the percentage of the children
    }
}

bt::Uint64 TorrentFileTreeModel::Node::bytesToDownload(const bt::TorrentInterface *tc)
{
    bt::Uint64 s = 0;

    if (!file) {
        // directory
        foreach (Node *n, children)
            s += n->bytesToDownload(tc);
    } else {
        if (!file->doNotDownload())
            s = file->getSize();
    }
    return s;
}

Qt::CheckState TorrentFileTreeModel::Node::checkState(const bt::TorrentInterface *tc) const
{
    if (!file) {
        bool found_checked = false;
        bool found_unchecked = false;
        // directory
        foreach (Node *n, children) {
            Qt::CheckState s = n->checkState(tc);
            if (s == Qt::PartiallyChecked)
                return s;
            else if (s == Qt::Checked)
                found_checked = true;
            else
                found_unchecked = true;

            if (found_checked && found_unchecked)
                return Qt::PartiallyChecked;
        }

        return found_checked ? Qt::Checked : Qt::Unchecked;
    } else {
        return file->doNotDownload() || file->getPriority() == ONLY_SEED_PRIORITY ? Qt::Unchecked : Qt::Checked;
    }
}

void TorrentFileTreeModel::Node::saveExpandedState(const QModelIndex &index, QSortFilterProxyModel *pm, QTreeView *tv, BEncoder *enc)
{
    if (file)
        return;

    enc->write(QByteArray("expanded"));
    enc->write((Uint32)(tv->isExpanded(pm->mapFromSource(index)) ? 1 : 0));

    int idx = 0;
    foreach (Node *n, children) {
        if (!n->file) {
            enc->write(n->name.toUtf8());
            enc->beginDict();
            n->saveExpandedState(index.model()->index(idx, 0), pm, tv, enc);
            enc->end();
        }
        ++idx;
    }
}

void TorrentFileTreeModel::Node::loadExpandedState(const QModelIndex &index, QSortFilterProxyModel *pm, QTreeView *tv, BDictNode *dict)
{
    if (file)
        return;

    if (!dict)
        return;

    BValueNode *v = dict->getValue(QByteArray("expanded"));
    if (v)
        tv->setExpanded(pm->mapFromSource(index), v->data().toInt() == 1);

    int idx = 0;
    foreach (Node *n, children) {
        if (!n->file) {
            BDictNode *d = dict->getDict(n->name.toUtf8());
            if (d)
                n->loadExpandedState(index.model()->index(idx, 0), pm, tv, d);
        }
        idx++;
    }
}

QString TorrentFileTreeModel::Node::path()
{
    if (!parent)
        return QString(); // the root node must not be included in the path

    if (file)
        return name;
    else
        return parent->path() + name + bt::DirSeparator();
}

TorrentFileTreeModel::TorrentFileTreeModel(bt::TorrentInterface *tc, DeselectMode mode, QObject *parent)
    : TorrentFileModel(tc, mode, parent)
    , root(nullptr)
    , emit_check_state_change(true)
{
    if (tc->getStats().multi_file_torrent)
        constructTree();
    else
        root = new Node(nullptr, tc->getStats().torrent_name, tc->getStats().total_chunks);
}

TorrentFileTreeModel::~TorrentFileTreeModel()
{
    delete root;
}

void TorrentFileTreeModel::constructTree()
{
    bt::Uint32 num_chunks = tc->getStats().total_chunks;
    if (!root)
        root = new Node(nullptr, tc->getUserModifiedFileName(), num_chunks);

    for (Uint32 i = 0; i < tc->getNumFiles(); ++i) {
        bt::TorrentFileInterface &tf = tc->getTorrentFile(i);
        root->insert(tf.getUserModifiedPath(), &tf, num_chunks);
    }
}

int TorrentFileTreeModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return 1;
    } else {
        Node *n = (Node *)parent.internalPointer();
        return n->children.count();
    }
}

int TorrentFileTreeModel::columnCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return 2;
    else
        return 2;
}

QVariant TorrentFileTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
        return QVariant();

    switch (section) {
    case 0:
        return i18n("File");
    case 1:
        return i18n("Size");
    default:
        return QVariant();
    }
}

QVariant TorrentFileTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    Node *n = (Node *)index.internalPointer();
    if (!n)
        return QVariant();

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.column()) {
        case 0:
            return n->name;
        case 1:
            if (tc->getStats().multi_file_torrent)
                return BytesToString(n->fileSize(tc));
            else
                return BytesToString(tc->getStats().total_bytes);
        default:
            return QVariant();
        }
    } else if (role == Qt::UserRole) // sorting
    {
        switch (index.column()) {
        case 0:
            return n->name;
        case 1:
            if (tc->getStats().multi_file_torrent)
                return n->fileSize(tc);
            else
                return tc->getStats().total_bytes;
        default:
            return QVariant();
        }
    } else if (role == Qt::DecorationRole && index.column() == 0) {
        // if this is an empty folder then we are in the single file case
        QMimeDatabase db;
        if (!n->file)
            return n->children.count() > 0 ? QIcon::fromTheme("folder") : QIcon::fromTheme(db.mimeTypeForFile(tc->getStats().torrent_name).iconName());
        else
            return QIcon::fromTheme(db.mimeTypeForFile(n->file->getPath()).iconName());
    } else if (role == Qt::CheckStateRole && index.column() == 0) {
        if (tc->getStats().multi_file_torrent)
            return n->checkState(tc);
    }

    return QVariant();
}

QModelIndex TorrentFileTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    Node *child = static_cast<Node *>(index.internalPointer());
    if (!child)
        return QModelIndex();

    Node *parent = child->parent;
    if (!parent)
        return QModelIndex();
    else
        return createIndex(parent->row(), 0, parent);
}

QModelIndex TorrentFileTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    Node *p = nullptr;

    if (!parent.isValid())
        return createIndex(row, column, root);
    else {
        p = static_cast<Node *>(parent.internalPointer());

        if (row >= 0 && row < p->children.count())
            return createIndex(row, column, p->children.at(row));
        else
            return QModelIndex();
    }
}

bool TorrentFileTreeModel::setCheckState(const QModelIndex &index, Qt::CheckState state)
{
    Node *n = static_cast<Node *>(index.internalPointer());
    if (!n)
        return false;

    if (!n->file) {
        bool reenable = false;
        if (emit_check_state_change) {
            reenable = true;
            emit_check_state_change = false;
        }

        for (int i = 0; i < n->children.count(); i++) {
            // recurse down the tree
            setCheckState(index.model()->index(i, 0), state);
        }

        if (reenable)
            emit_check_state_change = true;
    } else {
        bt::TorrentFileInterface *file = n->file;
        if (state == Qt::Checked) {
            if (file->getPriority() == ONLY_SEED_PRIORITY)
                file->setPriority(NORMAL_PRIORITY);
            else
                file->setDoNotDownload(false);
        } else {
            if (mode == KEEP_FILES)
                file->setPriority(ONLY_SEED_PRIORITY);
            else
                file->setDoNotDownload(true);
        }
        dataChanged(createIndex(index.row(), 0), createIndex(index.row(), columnCount(index) - 1));

        QModelIndex parent = index.parent();
        if (parent.isValid())
            dataChanged(parent, parent); // parent needs to be updated to
    }

    if (emit_check_state_change)
        checkStateChanged();
    return true;
}

void TorrentFileTreeModel::modifyPathOfFiles(Node *n, const QString &path)
{
    for (int i = 0; i < n->children.count(); i++) {
        Node *c = n->children.at(i);
        if (!c->file) // another directory, continue recursively
            modifyPathOfFiles(c, path + c->name + bt::DirSeparator());
        else
            c->file->setUserModifiedPath(path + c->name);
    }
}

bool TorrentFileTreeModel::setName(const QModelIndex &index, const QString &name)
{
    Node *n = static_cast<Node *>(index.internalPointer());
    if (!n || name.isEmpty() || name.contains(bt::DirSeparator()))
        return false;

    if (!tc->getStats().multi_file_torrent) {
        // single file case so we only need to change the user modified name
        tc->setUserModifiedFileName(name);
        n->name = name;
        dataChanged(index, index);
        return true;
    }

    if (!n->file) {
        // we are in a directory
        n->name = name;
        if (!n->parent) {
            // toplevel directory name has changed
            tc->setUserModifiedFileName(name);
        }

        dataChanged(index, index);
        // modify the path of all files
        modifyPathOfFiles(n, n->path());
        return true;
    } else {
        n->name = name;
        n->file->setUserModifiedPath(n->path());
        dataChanged(index, index);
        return true;
    }
}

bool TorrentFileTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;

    if (role == Qt::CheckStateRole)
        return setCheckState(index, static_cast<Qt::CheckState>(value.toInt()));
    else if (role == Qt::EditRole)
        return setName(index, value.toString());

    return false;
}

void TorrentFileTreeModel::checkAll()
{
    if (tc->getStats().multi_file_torrent)
        setData(index(0, 0, QModelIndex()), Qt::Checked, Qt::CheckStateRole);
}

void TorrentFileTreeModel::uncheckAll()
{
    if (tc->getStats().multi_file_torrent)
        setData(index(0, 0, QModelIndex()), Qt::Unchecked, Qt::CheckStateRole);
}

void TorrentFileTreeModel::invertCheck()
{
    if (!tc->getStats().multi_file_torrent)
        return;

    invertCheck(index(0, 0, QModelIndex()));
}

void TorrentFileTreeModel::invertCheck(const QModelIndex &idx)
{
    Node *n = static_cast<Node *>(idx.internalPointer());
    if (!n)
        return;

    if (!n->file) {
        for (int i = 0; i < n->children.count(); i++) {
            // recurse down the tree
            invertCheck(idx.model()->index(i, 0));
        }
    } else {
        if (n->file->doNotDownload())
            setData(idx, Qt::Checked, Qt::CheckStateRole);
        else
            setData(idx, Qt::Unchecked, Qt::CheckStateRole);
    }
}

bt::Uint64 TorrentFileTreeModel::bytesToDownload()
{
    if (tc->getStats().multi_file_torrent)
        return root->bytesToDownload(tc);
    else
        return tc->getStats().total_bytes;
}

QByteArray TorrentFileTreeModel::saveExpandedState(QSortFilterProxyModel *pm, QTreeView *tv)
{
    if (!tc->getStats().multi_file_torrent)
        return QByteArray();

    QByteArray data;
    BEncoder enc(std::make_unique<BEncoderBufferOutput>(data));
    enc.beginDict();
    root->saveExpandedState(index(0, 0, QModelIndex()), pm, tv, &enc);
    enc.end();
    return data;
}

void TorrentFileTreeModel::loadExpandedState(QSortFilterProxyModel *pm, QTreeView *tv, const QByteArray &state)
{
    if (!tc->getStats().multi_file_torrent)
        return;

    BDecoder dec(state, false, 0);
    const std::unique_ptr<BDictNode> dict = dec.decodeDict();
    if (dict) {
        root->loadExpandedState(index(0, 0, QModelIndex()), pm, tv, dict.get());
    }
}

bt::TorrentFileInterface *TorrentFileTreeModel::indexToFile(const QModelIndex &idx)
{
    if (!idx.isValid())
        return nullptr;

    Node *n = (Node *)idx.internalPointer();
    if (!n)
        return nullptr;

    return n->file;
}

QString TorrentFileTreeModel::dirPath(const QModelIndex &idx)
{
    if (!idx.isValid())
        return QString();

    Node *n = (Node *)idx.internalPointer();
    if (!n || n == root)
        return QString();

    QString ret = n->name;
    do {
        n = n->parent;
        if (n && n->parent)
            ret = n->name + bt::DirSeparator() + ret;
    } while (n);

    return ret;
}

void TorrentFileTreeModel::changePriority(const QModelIndexList &indexes, bt::Priority newpriority)
{
    foreach (const QModelIndex &idx, indexes) {
        Node *n = (Node *)idx.internalPointer();
        if (!n)
            continue;

        setData(idx, newpriority, Qt::UserRole);
    }
}
}

#include "moc_torrentfiletreemodel.cpp"
