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
#ifndef KTTORRENTFILELISTMODEL_H
#define KTTORRENTFILELISTMODEL_H

#include "torrentfilemodel.h"

namespace kt
{

	/**
	 * Model for displaying file trees of a torrent
	 * @author Joris Guisson
	*/
	class TorrentFileListModel : public TorrentFileModel
	{
		Q_OBJECT
	public:
		TorrentFileListModel(bt::TorrentInterface* tc,DeselectMode mode,QObject* parent);
		virtual ~TorrentFileListModel();
		
		virtual int rowCount(const QModelIndex & parent) const override;
		virtual int columnCount(const QModelIndex & parent) const override;
		virtual QVariant headerData(int section, Qt::Orientation orientation,int role) const override;
		virtual QVariant data(const QModelIndex & index, int role) const override;
		virtual QModelIndex parent(const QModelIndex & index) const override;
		virtual QModelIndex index(int row,int column,const QModelIndex & parent) const override;
		virtual bool setData(const QModelIndex & index, const QVariant & value, int role) override;
		virtual void checkAll() override;
		virtual void uncheckAll() override;
		virtual void invertCheck() override;
		virtual bt::Uint64 bytesToDownload() override;
		virtual bt::TorrentFileInterface* indexToFile(const QModelIndex & idx) override;
		virtual QString dirPath(const QModelIndex & idx) override;
		virtual void changePriority(const QModelIndexList & indexes,bt::Priority newpriority) override;

	private: 
		void invertCheck(const QModelIndex & idx);
	};

}

#endif
