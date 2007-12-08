/***************************************************************************
 *   Copyright (C) 2007 by Joris Guisson   <joris.guisson@gmail.com>       *
 *   Copyright (C) 2007 by Lukas Appelhans <l.appelhans@gmx.de>            *
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

#include <interfaces/chunkdownloadinterface.h>

#include <QTreeWidgetItem>

class ChunkDownloadViewItem : public QTreeWidgetItem
{
    public:
	    ChunkDownloadViewItem(QTreeWidget* cdv,bt::ChunkDownloadInterface* cd);
	    virtual ~ChunkDownloadViewItem();

	    void update(bool init = false);

	    bool operator < (const QTreeWidgetItem & other) const;

    private:
	    bt::ChunkDownloadInterface* cd;
	    bt::ChunkDownloadInterface::Stats stats;
};