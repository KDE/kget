/* This file is part of the KDE project
   
   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#ifndef _MAINVIEW_H
#define _MAINVIEW_H

#include <qmap.h>
#include <qwidget.h>

#include <klistview.h>

#include "core/observer.h"

class KActionCollection;
class MainView;
class GroupHandler;
class TransferHandler;
class TransferGroupHandler;

class TransferGroupItem : public QListViewItem, public TransferGroupObserver
{
public:
    TransferGroupItem(MainView * parent, TransferGroupHandler * group);
    ~TransferGroupItem();

    //TransferGroupObserver
    void groupChangedEvent(TransferGroupHandler * group);
    void addedTransferEvent(TransferHandler * transfer);
    void removedTransferEvent(TransferHandler * transfer);
    void deletedEvent(TransferGroupHandler * group);

    void updateContents(bool updateAll=false);

    TransferGroupHandler * group() const {return m_group;}
    MainView * view() const {return m_view;}

    void updatePixmaps();
    void paintCell(QPainter * p, const QColorGroup & cg, int column, int width, int align);

private:
    TransferGroupHandler * m_group;
    MainView * m_view;

    QPixmap * m_topGradient;
    QPixmap * m_bottomGradient;
};

class TransferItem : public QListViewItem, public TransferObserver
{
public:
    TransferItem(TransferGroupItem * parent, TransferHandler * transfer);
    ~TransferItem(){}

    //Transfer observer virtual functions
    void transferChangedEvent(TransferHandler * transfer);
    void deletedEvent(TransferHandler * transfer) {}

    void updateContents(bool updateAll=false);

    TransferHandler * transfer() const {return m_transfer;}
    MainView * view() const {return m_view;}

    void paintCell(QPainter * p, const QColorGroup & cg, int column, int width, int align);

private:
    TransferHandler * m_transfer;
    MainView * m_view;
};

class MainView : public KListView, public ModelObserver
{
    Q_OBJECT

    public:
    MainView( QWidget * parent, const char * name = 0 );
    ~MainView();

    //Model observer virtual functions
    void addedTransferGroupEvent(TransferGroupHandler * group);

    //KAction setup
    void setupActions( KActionCollection * a );

    protected:
    void paletteChange ();

    public slots:
    void slotRightButtonClicked( QListViewItem *, const QPoint &, int);

    //from "kget menu" popup
    void slotNewTransfer();
    //from "transfer operations" popup
    void slotResumeItems();
    void slotStopItems();
    void slotRemoveItems();

    void slotSetPriority( int );
    void slotSetGroup( int );

    private:
    QValueList<TransferHandler *> getSelectedList();

    KPopupMenu * m_popup;
    KActionCollection * ac;
};

#endif
