/* This file is part of the KDE project

   Copyright (C) 2006 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2007 by Javier Goday <jgoday@gmail.com>
   Copyright (C) 2010 Matthias Fuchs <mat69@gmx.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef TRANSFERSVIEWDELEGATE_H
#define TRANSFERSVIEWDELEGATE_H

#include <KExtendableItemDelegate>

#include <QModelIndex>
#include <QToolButton>

class QHBoxLayout;
class QButtonGroup;

class TransfersViewDelegate;

class GroupStatusButton : public QToolButton
{
    Q_OBJECT

public:
    GroupStatusButton(const QModelIndex &index, QWidget *parent);

protected:
    void checkStateSet() override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void timerEvent(QTimerEvent *event) override;

private:
    enum { None, Selecting, Deselecting, Blinking, BlinkingExiting } m_status;
    QModelIndex m_index;

    int m_timerId;
    int m_iconSize;

    float m_gradientId;
};

class GroupStatusEditor : public QWidget
{
    Q_OBJECT

public:
    GroupStatusEditor(const QModelIndex &index, QWidget *parent);

    void setRunning(bool running);
    bool isRunning();

private Q_SLOTS:
    void slotStatusChanged();

Q_SIGNALS:
    void changedStatus(GroupStatusEditor *editor);

private:
    QModelIndex m_index;

    QHBoxLayout *m_layout;

    QButtonGroup *m_btGroup;
    GroupStatusButton *m_startBt;
    GroupStatusButton *m_stopBt;
};

/**
 * The BasicTransfersViewDelegate handles the setting of the status of a group
 */
class BasicTransfersViewDelegate : public KExtendableItemDelegate
{
    Q_OBJECT

public:
    BasicTransfersViewDelegate(QAbstractItemView *parent);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

private Q_SLOTS:
    virtual void slotGroupStatusChanged(GroupStatusEditor *editor);
};

class TransfersViewDelegate : public BasicTransfersViewDelegate
{
    Q_OBJECT

public:
    TransfersViewDelegate(QAbstractItemView *parent);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    void drawFocus(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect) const;

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;
};

#endif
