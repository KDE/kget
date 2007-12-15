/* This file is part of the KDE project

   Copyright (C) 2007 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef BTFILETREEVIEW_H
#define BTFILETREEVIEW_H

#include "iwfiletreemodel.h"

#include <QTreeView>

#include <util/constants.h>


class KMenu;

namespace bt
{
    class TorrentInterface;
    class TorrentControl;
}

class BTFileTreeView : public QTreeView
{
    Q_OBJECT
    public:
        BTFileTreeView(bt::TorrentInterface *tc, QWidget * parent);

    private slots:
        void contextMenuEvent(QContextMenuEvent * e);
        void open();
	void downloadFirst();
	void downloadLast();
	void downloadNormal();
	void doNotDownload();
	void deleteFiles();

    private:
        void changePriority(bt::Priority newpriority);

        kt::IWFileTreeModel *fileTreeModel;
        bt::TorrentInterface *m_tc;

        KMenu *contextMenu;

	QAction* open_action;
	QAction* download_first_action;
	QAction* download_normal_action;
	QAction* download_last_action;
	QAction* dnd_action;
	QAction* delete_action;

        QString preview_path;
};

#endif
