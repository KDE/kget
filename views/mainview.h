#ifndef _MAINVIEW_H
#define _MAINVIEW_H

#include <qwidget.h>
#include <klistview.h>
#include <qmap.h>

#include "viewinterface.h"

class Transfer;
class MainView;

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
	virtual void schedulerAddedItems( TransferList );
	virtual void schedulerRemovedItems( TransferList );
	virtual void schedulerChangedItems( TransferList );
	virtual void schedulerStatus( GlobalStatus * );
    
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
    QMap<Transfer *, MainViewItem *> transfersMap;
};

#endif
