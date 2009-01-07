/* This file is part of the KDE project

   Copyright (C) 2002 Patrick Charbonnier <pch@freeshell.org>
   Based On Caitoo v.0.7.3 (c) 1998 - 2000, Matej Koss

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef DROPTARGET_H
#define DROPTARGET_H

#include <QWidget>
#include <QDragEnterEvent>
#include <QMouseEvent>
#include <QDropEvent>
#include <QCloseEvent>

#include "core/observer.h"

class QAction;
class QTimer;
class KMenu;

class MainWindow;
class DropTargetModelObserver;
class DropTargetGroupObserver;
class DropTargetTransferObserver;
    
class DropTarget : public QWidget
{
Q_OBJECT

    friend class DropTargetModelObserver;
    friend class DropTargetGroupObserver;
    friend class DropTargetTransferObserver;
public:
    DropTarget(MainWindow * parent);
    ~DropTarget();

    void playAnimationShow();
    void playAnimationHide();
    void playAnimationSync();
    void setDropTargetVisible( bool shown, bool internal = true );

protected:
    // drag and drop
    void dragEnterEvent(QDragEnterEvent *);
    void dropEvent(QDropEvent *);

    // handle quit events as hide events
    void closeEvent( QCloseEvent * );

    void mousePressEvent(QMouseEvent * e);
    void mouseReleaseEvent(QMouseEvent * e);
    void mouseDoubleClickEvent(QMouseEvent * e);
    void mouseMoveEvent(QMouseEvent * e);

    // paint the drop target
    void paintEvent(QPaintEvent*);

private slots:
    void toggleSticky();
    void toggleMinimizeRestore();
    void slotStartStopToggled( bool );
    void slotAnimateShow();
    void slotAnimateHide();
    void slotAnimateSync();
    void slotToolTipUpdate();
    void slotClose();

private:
    KMenu * popupMenu;
    MainWindow * parentWidget;
    QTimer * animTimer;
    QPixmap cachedPixmap;

    QAction * pop_sticky;
    QAction * pop_show;

    QPoint position;

    QString tooltipText;

    int dx;
    int dy;
    bool isdragging;
    bool showInformation;

    float ani_y, ani_vy;
};

/**
 * Checks every transfer for a percent change to update the mainwindow title
 */
class DropTargetTransferObserver : public QObject, public TransferObserver
{
    Q_OBJECT
    public:
        DropTargetTransferObserver(DropTarget *window);
        virtual ~DropTargetTransferObserver(){}

        virtual void transferChangedEvent(TransferHandler * transfer);

//        virtual void deleteEvent(TransferHandler * transfer);

    private:
        DropTarget *m_window;
};

/**
* Used to update the mainwindow caption when the groups percents change
*/
class DropTargetGroupObserver : public QObject, public TransferGroupObserver
{
    Q_OBJECT
    public:
        DropTargetGroupObserver(DropTarget *window);
        virtual ~DropTargetGroupObserver() {}

        virtual void groupChangedEvent(TransferGroupHandler * group);

        virtual void addedTransferEvent(TransferHandler * transfer, TransferHandler * after);

        virtual void removedTransferEvent(TransferHandler * transfer);
/**
        virtual void movedTransferEvent(TransferHandler * transfer, TransferHandler * after);**/

    private:
        DropTarget *m_window;
        DropTargetTransferObserver *m_transferObserver;
};

class DropTargetModelObserver : public QObject, public ModelObserver
{
    Q_OBJECT
    public:
        DropTargetModelObserver(DropTarget *window);
        virtual ~DropTargetModelObserver (){}

        virtual void addedTransferGroupEvent(TransferGroupHandler * group);

        virtual void removedTransferGroupEvent(TransferGroupHandler * group);

    private:
        DropTarget *m_window;
        DropTargetGroupObserver *m_groupObserver;
};
#endif                          // _DROPTARGET_H
