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
#include <QWidget>
#include <QList>
#include <QPixmap>
#include <QTimerEvent>

//Temporary!
#include <QPushButton>

#include "core/observer.h"

class Sidebar;

class SidebarBox : public QWidget
{
    Q_OBJECT
    public:
        SidebarBox( Sidebar * sidebar, int itemHeight, bool enableAnimations = false);
        ~SidebarBox();

        void addChild(SidebarBox *);
        void removeChild(SidebarBox *);

        void showChildren( bool show = true );

        void setHighlighted( bool highlighted = true );
        void setSelected( bool selected = true );

        /**
        * This method updates all the pixmaps that are used to draw the items
        */
        void updatePixmaps();

        void timerEvent(); //This function is called only by the Sidebar

    protected:
        virtual void paintEvent ( QPaintEvent * event );

        void setShown(bool show = true);
        void setAnimationEnabled(bool enable = true);

        void paletteChange ( const QPalette & oldPalette );

        Sidebar *    m_sidebar;
        bool         m_enableAnimations;
        bool         m_isHighlighted;
        bool         m_isShown;
        int          m_itemHeight;    //item height when shown
        bool         m_showChildren;

        //Pixmaps
        QPixmap *    m_pixFunsel;
        QPixmap *    m_pixFsel;
        QPixmap *    m_pixTgrad;
        QPixmap *    m_pixBgrad;
        QPixmap *    m_pixPlus;
        QPixmap *    m_pixMinus;

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
        virtual void paintEvent ( QPaintEvent * event );

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
        virtual void paintEvent ( QPaintEvent * event );

    private:
        TransferHandler * m_transfer;
};

class Sidebar : public QWidget, public ModelObserver
{
    Q_OBJECT
    friend class SidebarBox;

    public:
        Sidebar( QWidget * parent = 0, const char * name = 0 );

        void insertItem( SidebarBox * box, SidebarBox * after=0 );
        void removeItem( SidebarBox * box );

        void startTimer( SidebarBox * item );
        void stopTimer( SidebarBox * item );

        //Methods reimplemented from the ModelObserver class
        void addedTransferGroupEvent(TransferGroupHandler * group);
        void removedTransferGroupEvent(TransferGroupHandler * group);

        void boxHighlighedEvent(SidebarBox *);
        void boxSelectedEvent(SidebarBox *);

    private:
        void timerEvent ( QTimerEvent * );

        int       m_numBoxes;

        int       m_timerId;
        const int m_timerInterval;

        QList<SidebarBox *> m_activeTimers;
        //TODO Make sure we need this list with qt4
        QList<SidebarBox *> m_timersToRemove;

        SidebarBox * m_highlightedBox;

        QVBoxLayout  * m_layout;
       DownloadsBox * m_downloadsBox;
       QMap<QString, GroupBox *> m_groupsMap;
};

#endif
