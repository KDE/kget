/** IMPORTANT: please keep PARTS of this file in sync with ktorrent! *******/

/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson   <joris.guisson@gmail.com>       *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/

#include <klocale.h>
#include <kicon.h>
#include <kmenu.h>
#include <kstandarddirs.h>
#include <kconfiggroup.h>
#include <QHeaderView>
#include <interfaces/peerinterface.h>
#include <torrent/ipblocklist.h>
#include <util/functions.h>
#include "peerview.h"

using namespace bt;

namespace kt
{
	static KIcon yes,no;
	
	PeerViewItem::PeerViewItem(PeerView* pv,PeerInterface* peer) : QTreeWidgetItem(pv,QTreeWidgetItem::UserType),peer(peer)
	{
		const PeerInterface::Stats & s = peer->getStats();

		// stuff that doesn't change
		setText(0,s.ip_address);
		setText(1,s.client);
		setIcon(7,s.dht_support ? yes : no);
		if (s.encrypted)
			setIcon(0,KIcon("ktencrypted"));

		// initialize the stats
		stats = s;
		update(true);
	}

	PeerViewItem::~PeerViewItem()
	{
	}
	
	bool PeerViewItem::operator < (const QTreeWidgetItem & other) const
	{
		const PeerViewItem & pvi = (const PeerViewItem &) other;
		const PeerInterface::Stats & s = stats;
		const PeerInterface::Stats & os = pvi.stats;
		switch (treeWidget()->sortColumn())
		{
		case 0: 
		case 1: 
			return QTreeWidgetItem::operator < (other);
		case 2: return s.download_rate < os.download_rate;
		case 3: return s.upload_rate < os.upload_rate;
		case 4: return s.choked < os.choked;
		case 5: return s.snubbed < os.snubbed;
		case 6: return s.perc_of_file < os.perc_of_file;
		case 7: return s.dht_support < os.dht_support;
		case 8: return s.aca_score < os.aca_score;
		case 9: return s.has_upload_slot < os.has_upload_slot;
		case 10: return s.num_down_requests+s.num_up_requests < os.num_down_requests+os.num_up_requests;
		case 11: return s.bytes_downloaded < os.bytes_downloaded; 
		case 12: return s.bytes_uploaded < os.bytes_uploaded;
		default:
			 return false;
		}
	}


	void PeerViewItem::update(bool init)
	{
		const PeerInterface::Stats & s = peer->getStats();
		KLocale* loc = KGlobal::locale();

		if (init || s.download_rate != stats.download_rate)
		{
			if (s.download_rate >= 103) // lowest "visible" speed, all below will be 0,0 Kb/s
				setText(2,KBytesPerSecToString(s.download_rate / 1024.0));
			else
				setText(2,"");
		}
		
		if (init || s.upload_rate != stats.upload_rate)
		{
			if (s.upload_rate >= 103) // lowest "visible" speed, all below will be 0,0 Kb/s
				setText(3,KBytesPerSecToString(s.upload_rate / 1024.0));
			else
				setText(3,"");
		}

		if (init || s.choked != stats.choked)
			setText(4,s.choked ? i18n("Yes") : i18n("No"));

		if (init || s.snubbed != stats.snubbed)
			setText(5,s.snubbed ? i18n("Yes") : i18n("No"));

		if (init || s.perc_of_file != stats.perc_of_file)
			setText(6,QString("%1 %").arg(loc->formatNumber(s.perc_of_file,2)));

		if (init || s.aca_score != stats.aca_score)
			setText(8,loc->formatNumber(s.aca_score,2));

		if (init || s.has_upload_slot != stats.has_upload_slot)
			setIcon(9,s.has_upload_slot ? yes : KIcon());

		if (init || s.num_down_requests != stats.num_down_requests || s.num_up_requests != stats.num_up_requests)
			setText(10,QString("%1 / %2").arg(s.num_down_requests).arg(s.num_up_requests));

		if (init || s.bytes_downloaded != stats.bytes_downloaded)
			setText(11,BytesToString(s.bytes_downloaded));

		if (init || s.bytes_uploaded != stats.bytes_uploaded)
			setText(12,BytesToString(s.bytes_uploaded));

		stats = s;
	}

	PeerView::PeerView(QWidget* parent) : QTreeWidget(parent)
	{
		setContextMenuPolicy(Qt::CustomContextMenu);
		setRootIsDecorated(false);
		setSortingEnabled(true);
		setAlternatingRowColors(true);

		QStringList columns;
		columns << i18n("IP Address")
			<< i18n("Client")
			<< i18n("Down Speed")
			<< i18n("Up Speed")
			<< i18n("Choked")
			<< i18n("Snubbed")
			<< i18n("Availability")
			<< i18n("DHT")
			<< i18n("Score")
			<< i18n("Upload Slot")
			<< i18n("Requests")
			<< i18n("Downloaded")
			<< i18n("Uploaded");

		setHeaderLabels(columns);
		
		context_menu = new KMenu(this);
		context_menu->addAction(KIcon("user-remove"),i18n("Kick Peer"),this,SLOT(kickPeer()));
		context_menu->addAction(KIcon("view-filter"),i18n("Ban Peer"),this,SLOT(banPeer()));
		connect(this,SIGNAL(customContextMenuRequested(const QPoint & )),
				this,SLOT(showContextMenu(const QPoint& )));
	}

	PeerView::~PeerView()
	{
	}
	
	void PeerView::showContextMenu(const QPoint& pos)
	{
		if (!currentItem())
			return;
		
		context_menu->popup(mapToGlobal(pos));
	}
	
	void PeerView::banPeer()
	{
		QTreeWidgetItem* cur = currentItem();
		if (cur)
		{
			PeerViewItem* pv = (PeerViewItem*)cur;
			pv->peer->kill();
		}
	}
	
	void PeerView::kickPeer()
	{
		QTreeWidgetItem* cur = currentItem();
		if (cur)
		{
			PeerViewItem* pv = (PeerViewItem*)cur;
			pv->peer->kill();
			IPBlocklist& filter = IPBlocklist::instance();
			filter.insert(pv->stats.ip_address,3);
		}
	}

	void PeerView::peerAdded(PeerInterface* peer)
	{
		items.insert(peer, new PeerViewItem(this,peer));
	}

	void PeerView::peerRemoved(PeerInterface* peer)
	{
		PeerViewItem* v = items.find(peer);
		if (v)
		{
			items.erase(peer);
			delete v;
		}
	}

	void PeerView::update()
	{
		 bt::PtrMap<PeerInterface*,PeerViewItem>::iterator i = items.begin();
		 while (i != items.end())
		 {
			 i->second->update(false);
			 i++;
		 }
	}

	void PeerView::removeAll()
	{
		items.clear();
		clear();
	}

	void PeerView::saveState(KSharedConfigPtr cfg)
	{
		KConfigGroup g = cfg->group("PeerView");
		QByteArray s = header()->saveState();
		g.writeEntry("state",s.toBase64());
	}
	
	void PeerView::loadState(KSharedConfigPtr cfg)
	{
		KConfigGroup g = cfg->group("PeerView");
		QByteArray s = QByteArray::fromBase64(g.readEntry("state",QByteArray()));
		if (!s.isNull())
			header()->restoreState(s);
	}
}

#include "peerview.moc"
