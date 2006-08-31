/* This file is part of the KDE project

   Copyright (C) 2006 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#ifndef _TRANSFERVIEWDELEGATE_H
#define _TRANSFERVIEWDELEGATE_H

#include <QItemDelegate>

class KMenu;

class TransfersViewDelegate : public QItemDelegate
{
    Q_OBJECT

    public:
        TransfersViewDelegate();

//         void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;

        bool editorEvent(QEvent * event, QAbstractItemModel * model, const QStyleOptionViewItem & option, const QModelIndex & index);

    private:
        KMenu * m_popup;
};

#endif
