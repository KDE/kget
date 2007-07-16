/* This file is part of the KDE project

   Copyright (C) 2006 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#ifndef TRANSFERSVIEWDELEGATE_H
#define TRANSFERSVIEWDELEGATE_H

#include <QItemDelegate>

class QHBoxLayout;
class QButtonGroup;
class QToolButton;

class KMenu;

class TransfersViewDelegate;

class GroupStatusEditor : public QWidget
{
    Q_OBJECT

    public:
        explicit GroupStatusEditor(const TransfersViewDelegate * delegate, QWidget * parent=0);

        void setRunning(bool running);
        bool isRunning();

    private slots:
        void slotStatusChanged(bool running);

    private:
        const TransfersViewDelegate * m_delegate;

        QHBoxLayout * m_layout;

        QButtonGroup * m_btGroup;
        QToolButton * m_startBt;
        QToolButton * m_stopBt;
};

class TransfersViewDelegate : public QItemDelegate
{
    Q_OBJECT

    friend class GroupStatusEditor;

    public:
        TransfersViewDelegate();

        void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;

        void drawFocus(QPainter * painter, const QStyleOptionViewItem & option, const QRect & rect) const;

        QSize sizeHint (const QStyleOptionViewItem & option, const QModelIndex & index) const;

        QWidget * createEditor(QWidget *parent, const QStyleOptionViewItem & option, const QModelIndex & index) const;

        bool editorEvent(QEvent * event, QAbstractItemModel * model, const QStyleOptionViewItem & option, const QModelIndex & index);

        void setEditorData(QWidget * editor, const QModelIndex & index) const;
        void setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const;

    private:
        KMenu * m_popup;
};

#endif
