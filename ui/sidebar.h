/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#ifndef _SIDEBAR_H
#define _SIDEBAR_H


#include <qpixmapcache.h>
#include <qmap.h>
#include <QVBoxLayout>
#include <QSpacerItem>
#include <QWidget>
#include <QList>
#include <QPixmap>
#include <QTimerEvent>

//Temporary!
#include <QPushButton>

#include <QToolButton>

#include "core/observer.h"

class Sidebar;

class Button : public QToolButton
{
    Q_OBJECT
    public:
        Button(QWidget * w);

        void paintEvent ( QPaintEvent * event );
};

class SidebarBox : public QWidget
{
    Q_OBJECT
    public:
        SidebarBox(Sidebar * sidebar, int headerHeight, int footerHeight, bool enableAnimations = false);
        ~SidebarBox();

        void addChild(SidebarBox *);
        void removeChild(SidebarBox *);

        void showChildren( bool show = true );
        void setHighlighted( bool highlighted = true );

        void repaintChildren();

        /**
        * This method updates all the pixmaps that are used to draw the items
        */
        void updatePixmaps();

        void timerEvent(); //This function is called only by the Sidebar

    protected:
        virtual void paintEvent ( QPaintEvent * event );
        void mouseMoveEvent ( QMouseEvent * event );
        void mousePressEvent ( QMouseEvent * event );
        void mouseReleaseEvent ( QMouseEvent * event );
        void mouseDoubleClickEvent ( QMouseEvent * event );
        void enterEvent ( QEvent * event );
        void leaveEvent ( QEvent * event );

        void setShown(bool show = true);
        void setAnimationEnabled(bool enable = true);

        void paletteChange ( const QPalette & oldPalette );

        bool         m_enableAnimations;
        bool         m_isHighlighted;
        bool         m_isShown;
        bool         m_showChildren;
        int          m_headerHeight;    //item's header height when shown
        int          m_footerHeight;    //item's footer height when shown

        //Pixmaps
        QPixmap *    m_pixFunsel;
        QPixmap *    m_pixFsel;
        QPixmap *    m_pixTgrad;
        QPixmap *    m_pixBgrad;
        QPixmap *    m_pixPlus;
        QPixmap *    m_pixMinus;

        Sidebar *    m_sidebar;

        QPainter *    m_painter;
        QWidget * m_headerSpacer;
        QWidget * m_footerSpacer;
        QVBoxLayout  * m_layout;

        QList<SidebarBox *> m_childBoxes;
};


class DownloadsBox : public SidebarBox
{
    public:
        DownloadsBox( Sidebar * sidebar );

    protected:
        virtual void paintEvent ( QPaintEvent * event );
};

class GroupBox : public SidebarBox,
                 public TransferGroupObserver,
                 public TransferObserver
{
    Q_OBJECT
    public:
        GroupBox( TransferGroupHandler * group, DownloadsBox * dbox, Sidebar * sidebar );
        ~GroupBox();

        TransferGroupHandler * group() {return m_group;}

        void groupChangedEvent(TransferGroupHandler * group);
        void addedTransferEvent(TransferHandler * transfer, TransferHandler * after);
        void transferChangedEvent(TransferHandler * transfer);

    protected:
        void paintEvent ( QPaintEvent * event );

    private:
        DownloadsBox * m_downloadsBox;
        TransferGroupHandler * m_group;
};

class TransferBox : public SidebarBox, public TransferObserver
{
    Q_OBJECT
    public:
        TransferBox( TransferHandler * group, GroupBox * gBox, Sidebar * sidebar );
        ~TransferBox();

        TransferHandler * transfer() {return m_transfer;}

        //TransferObserver stuff
        void transferChangedEvent(TransferHandler * transfer);

        GroupBox * m_groupBox;

    protected:
        void paintEvent ( QPaintEvent * event );

    private:
        TransferHandler * m_transfer;
};

class Sidebar : public QWidget, public ModelObserver
{
    Q_OBJECT
    friend class SidebarBox;

    public:
        Sidebar( QWidget * parent = 0, const char * name = 0 );

        void startTimer( SidebarBox * item );
        void stopTimer( SidebarBox * item );

        //Methods reimplemented from the ModelObserver class
        void addedTransferGroupEvent(TransferGroupHandler * group);
        void removedTransferGroupEvent(TransferGroupHandler * group);

        void boxHighlightedEvent(SidebarBox *);
        void boxSelectedEvent(SidebarBox *, bool selected);

    private:
        void timerEvent ( QTimerEvent * );

        int       m_timerId;
        const int m_timerInterval;

        QList<SidebarBox *> m_activeTimers;
        //TODO Make sure we need this list with qt4
        QList<SidebarBox *> m_timersToRemove;

        QList<SidebarBox *> m_highlightedBoxes;

        QVBoxLayout  * m_layout;
        DownloadsBox * m_downloadsBox;
        QMap<QString, GroupBox *> m_groupsMap;
};

#endif
