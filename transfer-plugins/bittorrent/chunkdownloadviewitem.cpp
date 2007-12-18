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

#include "chunkdownloadviewitem.h"

#include <util/functions.h>
#include <interfaces/torrentfileinterface.h>

using namespace bt;

ChunkDownloadViewItem::ChunkDownloadViewItem(QTreeWidget* cdv,ChunkDownloadInterface* cd,bt::TorrentInterface* tc) 
    : QTreeWidgetItem(cdv,QTreeWidgetItem::UserType),cd(cd)
{
    cd->getStats(stats);
    setText(0,QString::number(stats.chunk_index));
    update(true);
    QString files;
    if (tc->getStats().multi_file_torrent)
    {
	int n = 0;
	for (Uint32 i = 0;i < tc->getNumFiles();i++)
	{
	    const bt::TorrentFileInterface & tf = tc->getTorrentFile(i);
	    if (stats.chunk_index >= tf.getFirstChunk() && stats.chunk_index <= tf.getLastChunk())
	    {
		if (n > 0)
		    files += "\n";
		
		files += tf.getPath();
		n++;
	    }
	}
	    setText(5,files);
    }
}

ChunkDownloadViewItem::~ChunkDownloadViewItem()
{
}

bool ChunkDownloadViewItem::operator < (const QTreeWidgetItem & other) const
{
    const ChunkDownloadViewItem & cdvi = (const ChunkDownloadViewItem &) other;
    const ChunkDownloadInterface::Stats & s = stats;
    const ChunkDownloadInterface::Stats & os = cdvi.stats;
    switch (treeWidget()->sortColumn())
    {
    case 0: return s.chunk_index < os.chunk_index;
    case 1: return s.pieces_downloaded < os.pieces_downloaded;
    case 2: return s.current_peer_id < os.current_peer_id;
    case 3: return s.download_speed < os.download_speed;
    case 4: return s.num_downloaders < os.num_downloaders;
    case 5: return text(5) < other.text(5);
    default:
        return false;
    }
}


void ChunkDownloadViewItem::update(bool init)
{
    if (!cd)
        return;

    ChunkDownloadInterface::Stats s;
    cd->getStats(s);

    if (init || s.pieces_downloaded != stats.pieces_downloaded)
	setText(1,QString("%1 / %2").arg(s.pieces_downloaded).arg(s.total_pieces));
    
    if (init || s.download_speed != stats.download_speed)
	setText(3,KBytesPerSecToString(s.download_speed / 1024.0));

    if (init || s.num_downloaders != stats.num_downloaders)
	setText(4,QString::number(s.num_downloaders));

    if (init || s.current_peer_id != stats.current_peer_id)
	setText(2,s.current_peer_id);

    stats = s;
}
