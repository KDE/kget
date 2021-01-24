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

#include "core/transfer.h"

class QAction;
class QTimer;
class QMenu;

class MainWindow;
    
class DropTarget : public QWidget
{
Q_OBJECT

public:
    DropTarget(MainWindow * parent);
    ~DropTarget() override;

    void playAnimationShow();
    void playAnimationHide();
    void playAnimationSync();
    void setDropTargetVisible( bool shown, bool internal = true );
    
protected:
    // drag and drop
    void dragEnterEvent(QDragEnterEvent *) override;
    void dropEvent(QDropEvent *) override;

    // handle quit events as hide events
    void closeEvent( QCloseEvent * ) override;

    void mousePressEvent(QMouseEvent * e) override;
    void mouseReleaseEvent(QMouseEvent * e) override;
    void mouseDoubleClickEvent(QMouseEvent * e) override;
    void mouseMoveEvent(QMouseEvent * e) override;
    void enterEvent(QEvent * event) override;
    void leaveEvent(QEvent * event) override;

    // paint the drop target
    void paintEvent(QPaintEvent*) override;
    
private slots:
    void toggleSticky();
    void toggleMinimizeRestore();
    void slotStartStopToggled( bool );
    void slotAnimateShow();
    void slotAnimateHide();
    void slotAnimateSync();
    void slotToolTipUpdate();
    void slotToolTipTimer();
    void slotClose();

private:
    QMenu * popupMenu = nullptr;
    MainWindow * parentWidget = nullptr;
    QTimer * animTimer = nullptr;
    QTimer * popupTimer = nullptr;
    QPixmap cachedPixmap;

    QAction * pop_sticky = nullptr;
    QAction * pop_show = nullptr;

    QPoint position;

    QString tooltipText;

    int dx;
    int dy;
    bool isdragging;
    bool showInformation;

    float ani_y, ani_vy;
};

#endif                          // _DROPTARGET_H
