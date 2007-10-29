/* This file is part of the KDE project

   Copyright (C) 2006 Dario Massarin <nekkar@libero.it>
   Adapt the kshortcutdialog use of the kextendableitemdelegate in kdelibs/kdeui/dialogs/

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#ifndef TRANSFERSVIEWDELEGATE_H
#define TRANSFERSVIEWDELEGATE_H

#include "kextendableitemdelegate.h"

#include <QItemDelegate>
#include <QToolButton>
#include <QModelIndex>

class QHBoxLayout;
class QButtonGroup;

class KMenu;

class TransferHandler;
class TransfersViewDelegate;

static const QString EXPANDABLE_TRANSFER_DETAILS_STYLE =
                        "QGroupBox{"
                            "border-width:1px;margin:6px;margin-left:30px;margin-right:50px;"
                            "border-style:solid;border-color:black;border-radius:10;"
                            "background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #FEFEFE, stop: 0.5 #CECECE, stop:1 #FEFEFE);"
                        "}"
                        "QGroupBox QLineEdit{"
                            "border: 1px solid #cecece;border-radius:1;background-color:#F2F2F2;"
                        "}";
static const QString EXPANDABLE_TRANSFER_DETAILS_TITLE_STYLE =
                        "QLabel{"
                            "color:#343434;width:100%;font-weight:bold;"
                            "subcontrol-position: top center;"
                        "}";

class GroupStatusButton : public QToolButton
{
    Q_OBJECT

    public:
        explicit GroupStatusButton(const QModelIndex & index, QWidget * parent=0);

    protected:
        void checkStateSet();
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
        QToolButton * m_startBt;
        GroupStatusButton * m_stopBt;
};

class TransfersViewDelegate : public KExtendableItemDelegate
{
    Q_OBJECT

    friend class GroupStatusEditor;

    public:
        TransfersViewDelegate(QAbstractItemView *parent);

        void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;

        void drawFocus(QPainter * painter, const QStyleOptionViewItem & option, const QRect & rect) const;

        QSize sizeHint (const QStyleOptionViewItem & option, const QModelIndex & index) const;

        QWidget * createEditor(QWidget *parent, const QStyleOptionViewItem & option, const QModelIndex & index) const;

        bool editorEvent(QEvent * event, QAbstractItemModel * model, const QStyleOptionViewItem & option, const QModelIndex & index);

        void setEditorData(QWidget * editor, const QModelIndex & index) const;
        void setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const;

    public slots:
        void closeExpandableDetails(const QModelIndex &index = QModelIndex());
        void itemActivated(QModelIndex index);

    private:
        QWidget *getDetailsWidgetForTransfer(TransferHandler *handler);

        KMenu * m_popup;
        QList<QModelIndex> m_editingIndexes;
        // QMap<TransferHandler *, QWidget *> m_transfersMap;
};

#endif

