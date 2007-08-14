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
#include <QToolButton>
#include <QModelIndex>

class QHBoxLayout;
class QButtonGroup;

class KMenu;

class TransfersViewDelegate;

class GroupStatusButton : public QToolButton
{
    Q_OBJECT

    public:
        GroupStatusButton(const QModelIndex & index, QWidget * parent=0);

    protected:
        void nextCheckState();
        void enterEvent(QEvent * event);
        void leaveEvent(QEvent * event);
        void paintEvent(QPaintEvent * event);
        void timerEvent(QTimerEvent *event);

    private:
        enum {None, Selecting, Deselecting, Blinking, BlinkingExiting} m_status;
        QModelIndex m_index;

        int m_timerId;
        int m_iconSize;

        float m_gradientId;
};

class GroupStatusEditor : public QWidget
{
    Q_OBJECT

    public:
        explicit GroupStatusEditor(const QModelIndex & index, const TransfersViewDelegate * delegate, QWidget * parent=0);

        void setRunning(bool running);
        bool isRunning();

    private slots:
        void slotStatusChanged(bool running);

    private:
        const TransfersViewDelegate * m_delegate;

        QModelIndex m_index;

        QHBoxLayout * m_layout;

        QButtonGroup * m_btGroup;
        GroupStatusButton * m_startBt;
        GroupStatusButton * m_stopBt;
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
