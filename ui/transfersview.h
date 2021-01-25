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

class QMenu;
class TransferHandler;

class TransfersView : public QTreeView
{
    Q_OBJECT

    public:
        TransfersView(QWidget * parent = nullptr);
        ~TransfersView() override;

        void setModel(QAbstractItemModel * model) override;

    private:
        void dropEvent(QDropEvent * event) override;
        void rowsInserted(const QModelIndex &, int, int) override;

    protected:
        void dragMoveEvent ( QDragMoveEvent * event ) override;
        void rowsAboutToBeRemoved(const QModelIndex & parent, int start, int end) override;

    public Q_SLOTS:
        void closeExpandableDetails(const QModelIndex &index = QModelIndex());
        void closeExpandableDetails(const QModelIndex &parent, int rowStart, int rowEnd);
        void slotItemActivated(const QModelIndex & index);
        void slotItemCollapsed(const QModelIndex & index);

    private Q_SLOTS:
        void toggleMainGroup();// show or hide the first group header if there's only one download group
        void slotShowHeaderMenu(const QPoint &point);
        void selectionChanged(const QItemSelection & selected, const QItemSelection & deselected) override;
        void slotHideSection(int logicalIndex);
        void slotSectionMoved(int logicalIndex, int oldVisualIndex, int newVisualIndex);
        void slotSaveHeader();
        void populateHeaderActions();

    private:
        QWidget *getDetailsWidgetForTransfer(TransferHandler *handler);

        QList<QModelIndex> m_editingIndexes;
        QMenu *m_headerMenu = nullptr;
};

#endif
