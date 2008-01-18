/* This file is part of the KDE project

   Copyright (C) 2007 by Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef TRANSFERHISTORY_H
#define TRANSFERHISTORY_H

#include <KDialog>
#include <QList>
#include <QDomElement>

#include "ui_transferhistory.h"
#include "rangetreewidget.h"

class QFileSystemWatcher;

class TransferHistory : public KDialog, Ui::TransferHistory
{
    Q_OBJECT

    public:
        TransferHistory(QWidget *parent = 0);

    private:
        enum RangeType {
            Date = 0,
            Size = 1
        };
        void hideEvent(QHideEvent *event);
        QList<QDomElement> defaultItems;
        bool save;
        QFileSystemWatcher *watcher;
        int m_rangeType;
        RangeTreeWidget *m_treeWidget;
        QVBoxLayout *m_verticalLayout;
        QComboBox *m_rangeTypeCombobox;
        QHBoxLayout *m_hboxLayout;
        KTreeWidgetSearchLine *m_searchBar;
        QAction *m_actionDelete_Selected;
        QAction *m_actionClear;
        QAction *m_actionDownload;
        QAction *m_openFile;
        QPushButton *m_clearButton;

    private slots:
        void slotDeleteTransfer();
        void slotAddTransfers();
        void slotClear();
        void slotWriteDefault();
        void slotDownload();
        void slotOpenFile();
        void contextMenuEvent(QContextMenuEvent *event);
        void slotLoadRangeType(int type = -1);
};

#endif
