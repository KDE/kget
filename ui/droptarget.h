/* This file is part of the KDE project

   Copyright (C) 2002 Patrick Charbonnier <pch@freeshell.org>
   Based On Caitoo v.0.7.3 (c) 1998 - 2000, Matej Koss

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef _DROPTARGET_H
#define _DROPTARGET_H

#include <QWidget>
#include <QDragEnterEvent>
#include <QMouseEvent>
#include <QDropEvent>
#include <QCloseEvent>

class QTimer;
class KMenu;

class KGet;

class DropTarget : public QWidget
{
Q_OBJECT

public:
    DropTarget(KGet * parent);
    ~DropTarget();

    void updateStickyState();
    void playAnimation();
    void playAnimationHide();
    void playAnimationSync();
    void setVisible( bool shown, bool internal = true );

protected:
    // drag and drop
    virtual void dragEnterEvent(QDragEnterEvent *);
    virtual void mouseDoubleClickEvent(QMouseEvent * e);
    virtual void dropEvent(QDropEvent *);

    // handle quit events as hide events
    virtual void closeEvent( QCloseEvent * );

    virtual void mouseMoveEvent(QMouseEvent *);
    virtual void mousePressEvent(QMouseEvent * e);
    virtual void mouseReleaseEvent(QMouseEvent *);

    // paint the drop target
    virtual void paintEvent(QPaintEvent*);

private slots:
    void toggleSticky();
    void toggleMinimizeRestore();
    void slotStartStopToggled( bool );
    void slotAnimate();
    void slotAnimateHide();
    void slotAnimateSync();
    void slotClose();

private:
    KMenu * popupMenu;
    QWidget * parentWidget;
    QTimer * animTimer;
    QPixmap targetBuffer;

    int pop_sticky;
    int pop_show;

    int dx;
    int dy;
    bool isdragging;

    float ani_y, ani_vy;
};

#endif                          // _DROPTARGET_H
