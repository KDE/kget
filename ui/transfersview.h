/* This file is part of the KDE project

   Copyright (C) 2006 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef TRANSFERSVIEW_H
#define TRANSFERSVIEW_H

#include <QTreeView>

class TransferHandler;

class TransfersView : public QTreeView
{
    Q_OBJECT

    public:
        TransfersView(QWidget * parent = 0);
        ~TransfersView();

        void setModel(QAbstractItemModel * model);
        QModelIndex indexFromTransferHandler(TransferHandler *handler);

    private:
        void dropEvent(QDropEvent * event);
        void rowsInserted(const QModelIndex &, int, int);
        void populateHeaderActions();

    protected:
        void dragMoveEvent ( QDragMoveEvent * event );
        void rowsAboutToBeRemoved(const QModelIndex & parent, int start, int end);

    private slots:
        void toggleMainGroup();// show or hide the first group header if there's only one download group
        void slotSetColumnVisible(int column);
};

#endif
