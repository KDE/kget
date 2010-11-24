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

class KMenu;
class TransferHandler;

class TransfersView : public QTreeView
{
    Q_OBJECT

    public:
        TransfersView(QWidget * parent = 0);
        ~TransfersView();

        void setModel(QAbstractItemModel * model);

    private:
        void dropEvent(QDropEvent * event);
        void rowsInserted(const QModelIndex &, int, int);
        void populateHeaderActions();

    protected:
        void dragMoveEvent ( QDragMoveEvent * event );
        void rowsAboutToBeRemoved(const QModelIndex & parent, int start, int end);

    public slots:
        void closeExpandableDetails(const QModelIndex &index = QModelIndex());
        void closeExpandableDetails(const QModelIndex &parent, int rowStart, int rowEnd);
        void slotItemActivated(const QModelIndex & index);
        void slotItemCollapsed(const QModelIndex & index);

    private slots:
        void toggleMainGroup();// show or hide the first group header if there's only one download group
        void slotShowHeaderMenu(const QPoint &point);
        void selectionChanged(const QItemSelection & selected, const QItemSelection & deselected);
        void slotHideSection(int logicalIndex);
        void slotSectionMoved(int logicalIndex, int oldVisualIndex, int newVisualIndex);
        void slotSaveHeader();

    private:
        QWidget *getDetailsWidgetForTransfer(TransferHandler *handler);

        QList<QModelIndex> m_editingIndexes;
        KMenu *m_headerMenu;
};

#endif
