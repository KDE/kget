/* This file is part of the KDE project
   
   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#ifndef _MAINVIEW_H
#define _MAINVIEW_H

#include <qwidget.h>
#include <klistview.h>
#include <qmap.h>

#include "viewinterface.h"

class Transfer;
class MainView;

class MainViewGroupItem : public QListViewItem
{
    public:
    MainViewGroupItem(MainView * parent, Group * g);

    void updateContents(bool updateAll=false);
    Group * getGroup() const {return group;}
    
    void paintCell(QPainter * p, const QColorGroup & cg, int column, int width, int align);
    
    private:
    Group * group;
    MainView * view;
};

class MainViewItem : public QListViewItem
{
    public:
    MainViewItem(MainView * parent, Transfer * t);

    void updateContents(bool updateAll=false);
    Transfer * getTransfer() const {return transfer;}    
    
    void paintCell(QPainter * p, const QColorGroup & cg, int column, int width, int align);
    
    private:
    Transfer * transfer;
    MainView * view;
};

class MainView : public KListView, public ViewInterface
{
    Q_OBJECT
    
    public:
    MainView( QWidget * parent, const char * name = 0 );
    ~MainView();
    
	virtual void schedulerCleared();
	virtual void schedulerAddedItems( const TransferList& );
	virtual void schedulerRemovedItems( const TransferList& );
	virtual void schedulerChangedItems( const TransferList& );
	virtual void schedulerStatus( GlobalStatus * );
    virtual void schedulerAddedGroups( const GroupList& );
    virtual void schedulerRemovedGroups( const GroupList& );
    virtual void schedulerChangedGroups( const GroupList& );
    
    public slots:
    void slotRightButtonClicked( QListViewItem *, const QPoint &, int);
	
	//from "kget menu" popup
	void slotNewTransfer();
    //from "transfer operations" popup
	void slotResumeItems();
	void slotStopItems();
	void slotRemoveItems();

	void slotSetPriority1();
	void slotSetPriority2();
	void slotSetPriority3();
	void slotSetPriority4();
	void slotSetPriority5();
	void slotSetPriority6();
	void slotSetGroup();
    
    
    private:
    TransferList getSelectedList();
    QMap<QString, MainViewGroupItem *> groupsMap;
    QMap<Transfer *, MainViewItem *> transfersMap;
};

#endif
