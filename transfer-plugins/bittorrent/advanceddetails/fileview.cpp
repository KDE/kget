/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson                                   *
 *   joris.guisson@gmail.com                                               *
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
#include "fileview.h"

#include <QFileInfo>
#include <QFileDialog>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QMenu>
#include <QSortFilterProxyModel>

#include <KConfigGroup>
#include <KIconLoader>
#include <KLocalizedString>
#include <KMessageBox>
#include <KRun>

#include <util/bitset.h>
#include <util/error.h>
#include <util/functions.h>
#include <interfaces/torrentinterface.h>
#include <interfaces/torrentfileinterface.h>
#include <util/log.h>
#include <util/timer.h>
#include <KSharedConfig>
#include "iwfiletreemodel.h"
#include "iwfilelistmodel.h"
	
using namespace bt;

namespace kt
{

	FileView::FileView(QWidget *parent) : QTreeView(parent),curr_tc(nullptr),model(nullptr)
	{
		setContextMenuPolicy(Qt::CustomContextMenu);
		setRootIsDecorated(false);
		setSortingEnabled(true);
		setAlternatingRowColors(true);
		setSelectionMode(QAbstractItemView::ExtendedSelection);
		setSelectionBehavior(QAbstractItemView::SelectRows);
		setUniformRowHeights(true);
		
		proxy_model = new QSortFilterProxyModel(this);
		proxy_model->setSortRole(Qt::UserRole);
		setModel(proxy_model);
		
		context_menu = new QMenu(this);
		open_action = context_menu->addAction(QIcon::fromTheme("document-open"),i18nc("Open file", "Open"),this,&FileView::open);
		context_menu->addSeparator();
		download_first_action = context_menu->addAction(i18n("Download first"),this,&FileView::downloadFirst);
		download_normal_action = context_menu->addAction(i18n("Download normally"),this,&FileView::downloadNormal);
		download_last_action = context_menu->addAction(i18n("Download last"),this,&FileView::downloadLast);
		context_menu->addSeparator();
		dnd_action = context_menu->addAction(i18n("Do Not Download"),this,&FileView::doNotDownload);
		delete_action = context_menu->addAction(i18n("Delete File(s)"),this,&FileView::deleteFiles);
		context_menu->addSeparator();
		move_files_action = context_menu->addAction(i18n("Move File"),this,&FileView::moveFiles);
		context_menu->addSeparator();
		collapse_action = context_menu->addAction(i18n("Collapse Folder Tree"),this,&FileView::collapseTree);
		expand_action = context_menu->addAction(i18n("Expand Folder Tree"),this,&FileView::expandTree);
		
		connect(this,&QWidget::customContextMenuRequested,
				this,&FileView::showContextMenu);
		connect(this,&QAbstractItemView::doubleClicked,
				this,&FileView::onDoubleClicked);
		
		setEnabled(false);
		show_list_of_files = false;
		redraw = false;
	}


	FileView::~FileView()
	{}

	void FileView::changeTC(bt::TorrentInterface* tc,KSharedConfigPtr cfg)
	{
		if (tc == curr_tc)
			return;
	
		if (model)
		{
			saveState(cfg);
			if (curr_tc)
				expanded_state_map[curr_tc] = model->saveExpandedState(proxy_model,this);
		}
		proxy_model->setSourceModel(nullptr);
		delete model;
		model = nullptr;
		curr_tc = tc;
		setEnabled(tc != nullptr);
		if (tc)
		{
			connect(tc,&TorrentInterface::missingFilesMarkedDND,
					this,&FileView::onMissingFileMarkedDND);
			
			if (show_list_of_files)
				model = new IWFileListModel(tc,this);
			else 
				model = new IWFileTreeModel(tc,this);
			
			proxy_model->setSourceModel(model);
			setRootIsDecorated(tc->getStats().multi_file_torrent);
			loadState(cfg);
			QMap<bt::TorrentInterface*,QByteArray>::iterator i = expanded_state_map.find(tc);
			if (i != expanded_state_map.end())
				model->loadExpandedState(proxy_model,this,i.value());
			else
				expandAll();
		}
		else
		{
			proxy_model->setSourceModel(nullptr);
			model = nullptr;
		}
	}
	
	void FileView::onMissingFileMarkedDND(bt::TorrentInterface* tc)
	{
		if (curr_tc == tc)
			model->missingFilesMarkedDND();
	}
		
	void FileView::showContextMenu(const QPoint & p)
	{
		const TorrentStats & s = curr_tc->getStats();
		
		QModelIndexList sel = selectionModel()->selectedRows();
		if (sel.count() == 0)
			return;
		
		if (sel.count() > 1)
		{
			download_first_action->setEnabled(true);
			download_normal_action->setEnabled(true);
			download_last_action->setEnabled(true);
			open_action->setEnabled(false);
			dnd_action->setEnabled(true);
			delete_action->setEnabled(true);
			context_menu->popup(mapToGlobal(p));
			move_files_action->setEnabled(true);
			collapse_action->setEnabled(!show_list_of_files);
			expand_action->setEnabled(!show_list_of_files);
			return;
		}
	
		QModelIndex item = proxy_model->mapToSource(sel.front());
		bt::TorrentFileInterface* file = model->indexToFile(item);

		download_first_action->setEnabled(false);
		download_last_action->setEnabled(false);
		download_normal_action->setEnabled(false);
		dnd_action->setEnabled(false);
		delete_action->setEnabled(false);
		
		if (!s.multi_file_torrent)
		{
			open_action->setEnabled(true);
			move_files_action->setEnabled(true);
			preview_path = curr_tc->getStats().output_path;
			collapse_action->setEnabled(false);
			expand_action->setEnabled(false);
		}
		else if (file)
		{
			move_files_action->setEnabled(true);
			collapse_action->setEnabled(false);
			expand_action->setEnabled(false);
			if (!file->isNull())
			{
				open_action->setEnabled(true);
				preview_path = file->getPathOnDisk();
				
				download_first_action->setEnabled(file->getPriority() != FIRST_PRIORITY);
				download_normal_action->setEnabled(file->getPriority() != NORMAL_PRIORITY);
				download_last_action->setEnabled(file->getPriority() != LAST_PRIORITY);
				dnd_action->setEnabled(file->getPriority() != ONLY_SEED_PRIORITY);
				delete_action->setEnabled(file->getPriority() != EXCLUDED);
			}
			else
			{
				open_action->setEnabled(false);
			}
		}
		else
		{
			move_files_action->setEnabled(false);
			download_first_action->setEnabled(true);
			download_normal_action->setEnabled(true);
			download_last_action->setEnabled(true);
			dnd_action->setEnabled(true);
			delete_action->setEnabled(true);
			open_action->setEnabled(true);
			preview_path = curr_tc->getDataDir() + model->dirPath(item);
			collapse_action->setEnabled(!show_list_of_files);
			expand_action->setEnabled(!show_list_of_files);
		}

		context_menu->popup(mapToGlobal(p));
	}
	
	void FileView::open()
	{
		new KRun(QUrl(preview_path), nullptr, true);
	}
	
	void FileView::changePriority(bt::Priority newpriority)
	{
		QModelIndexList sel = selectionModel()->selectedRows(2);
		for (QModelIndexList::iterator i = sel.begin();i != sel.end();++i)
			*i = proxy_model->mapToSource(*i);
		
		model->changePriority(sel,newpriority);
		proxy_model->invalidate();
	}


	void FileView::downloadFirst()
	{
		changePriority(FIRST_PRIORITY);
	}

	void FileView::downloadLast()
	{
		changePriority(LAST_PRIORITY);
	}

	void FileView::downloadNormal()
	{
		changePriority(NORMAL_PRIORITY);
	}

	void FileView::doNotDownload()
	{
		changePriority(ONLY_SEED_PRIORITY);
	}

	void FileView::deleteFiles()
	{
		QModelIndexList sel = selectionModel()->selectedRows();
		Uint32 n = sel.count();
		if (n == 1) // single item can be a directory
		{
			if (!model->indexToFile(proxy_model->mapToSource(sel.front())))
				++n;
		} 
			
		QString msg = i18np("You will lose all data in this file, are you sure you want to do this?",
                            "You will lose all data in these files, are you sure you want to do this?", n);
				
		if (KMessageBox::warningYesNo(nullptr, msg) == KMessageBox::Yes)
			changePriority(EXCLUDED);
	}
	
	void FileView::moveFiles()
	{
		if (curr_tc->getStats().multi_file_torrent)
		{
			QModelIndexList sel = selectionModel()->selectedRows();
			QMap<bt::TorrentFileInterface*,QString> moves;
			
			QString dir = QFileDialog::getExistingDirectory(this, i18n("Select a directory to move the data to"));
			if (dir.isNull())
				return;
			
			foreach (const QModelIndex &idx,sel)
			{
				bt::TorrentFileInterface* tfi = model->indexToFile(proxy_model->mapToSource(idx));
				if (!tfi)
					continue;
			
				moves.insert(tfi,dir);
			}
			
			if (moves.count() > 0)
			{
				curr_tc->moveTorrentFiles(moves);
			}
		}
		else
		{
			QString dir = QFileDialog::getExistingDirectory(this, i18n("Select a directory to move the data to"));
			if (dir.isNull())
				return;
		
			curr_tc->changeOutputDir(dir,bt::TorrentInterface::MOVE_FILES);
		}
	}
	
	void FileView::expandCollapseTree(const QModelIndex& idx, bool expand) 
	{
		int rowCount = proxy_model->rowCount(idx);
		for (int i = 0; i < rowCount; i++) 
		{
			const QModelIndex& ridx = proxy_model->index(i, 0, idx);
			if (proxy_model->hasChildren(ridx))
				expandCollapseTree(ridx, expand);
		}
		setExpanded(idx, expand);
	}

	void FileView::expandCollapseSelected(bool expand) 
	{
		QModelIndexList sel = selectionModel()->selectedRows();
		for (QModelIndexList::iterator i = sel.begin(); i != sel.end(); ++i) 
		{
			if (proxy_model->hasChildren(*i))
				expandCollapseTree(*i, expand);
		}
	}

	void FileView::collapseTree() 
	{
		expandCollapseSelected(false);
	}

	void FileView::expandTree() 
	{
		expandCollapseSelected(true);
	}

	void FileView::onDoubleClicked(const QModelIndex & index)
	{
		if (!curr_tc)
			return;
		
		const TorrentStats & s = curr_tc->getStats();
		
		if (s.multi_file_torrent)
		{
			bt::TorrentFileInterface* file = model->indexToFile(proxy_model->mapToSource(index));
			if (!file)
			{
				// directory
				new KRun(QUrl(curr_tc->getDataDir() + model->dirPath(proxy_model->mapToSource(index))), nullptr, true);
			}
			else
			{
				// file
				new KRun(QUrl(file->getPathOnDisk()), nullptr, true);
			}
		}
		else
		{
			new KRun(QUrl(curr_tc->getStats().output_path), nullptr, true);
		}
	}
	
	void FileView::saveState(KSharedConfigPtr cfg)
	{
		if (!model)
			return;
		
		KConfigGroup g = cfg->group("FileView");
		QByteArray s = header()->saveState();
		g.writeEntry("state",s.toBase64());
	}
	
	void FileView::loadState(KSharedConfigPtr cfg)
	{
		KConfigGroup g = cfg->group("FileView");
		QByteArray s = QByteArray::fromBase64(g.readEntry("state",QByteArray()));
		if (!s.isNull())
		{
			QHeaderView* v = header();
			v->restoreState(s);
			sortByColumn(v->sortIndicatorSection(),v->sortIndicatorOrder());
		}
	}

	void FileView::update()
	{
		if (model)
			model->update();
		
		if (redraw)
		{
			scheduleDelayedItemsLayout();
			redraw = false;
		}
	}
	
	void FileView::onTorrentRemoved(bt::TorrentInterface* tc)
	{
		expanded_state_map.remove(tc);
	}
	
	void FileView::setShowListOfFiles(bool on,KSharedConfigPtr cfg)
	{
		if (show_list_of_files == on)
			return;
		
		show_list_of_files = on;
		if (!model || !curr_tc)
			return;
		
		saveState(cfg);
		expanded_state_map[curr_tc] = model->saveExpandedState(proxy_model,this);
		
		proxy_model->setSourceModel(nullptr);
		delete model;
		model = nullptr;
			
		if (show_list_of_files)
			model = new IWFileListModel(curr_tc,this);
		else 
			model = new IWFileTreeModel(curr_tc,this);
			
		proxy_model->setSourceModel(model);
		setRootIsDecorated(curr_tc->getStats().multi_file_torrent);
		loadState(cfg);
		QMap<bt::TorrentInterface*,QByteArray>::iterator i = expanded_state_map.find(curr_tc);
		if (i != expanded_state_map.end())
			model->loadExpandedState(proxy_model,this,i.value());
		else
			expandAll();

		collapse_action->setEnabled(!show_list_of_files);
		expand_action->setEnabled(!show_list_of_files);
	}
	
	bool FileView::viewportEvent(QEvent *event)
	{
		executeDelayedItemsLayout();
		return QTreeView::viewportEvent(event);
	}
	
	void FileView::filePercentageChanged(bt::TorrentFileInterface* file,float percentage)
	{
		if (model)
			model->filePercentageChanged(file,percentage);
	}
	
	void FileView::filePreviewChanged(bt::TorrentFileInterface* file,bool preview)
	{
		if (model)
			model->filePreviewChanged(file,preview);
	}
	
	void FileView::dataChanged(const QModelIndex & topLeft, const QModelIndex & bottomRight)
	{
        Q_UNUSED(topLeft)
        Q_UNUSED(bottomRight)
		redraw = true;
	}
}


