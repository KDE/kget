#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include <qobject.h>
#include <qvaluelist.h>
#include <kurl.h>

class QString;

class Transfer;

class GlobalStatus
{
public:
	// not yet used
	QDateTime timeStamp;
	struct {
		QString interface;
		float speed;
		float maxSpeed;
		float minSpeed;
	} connection;
	struct {
		float totalSize;
		float percentage;
		int transfersNumber;
	} files;
	QStringList others;
};



class Scheduler : public QObject
{
QOBJECT
public:
	Scheduler();
	~Scheduler();
	
	enum Operation {};
	
signals:
	void addedItems(QValueList<Transfer *>);
	void removedItems(QValueList<Transfer *>);
	void changedItems(QValueList<Transfer *>);
	void clear();
	void globalStatus(GlobalStatus *);
	
public slots:
	void slotNewURLs(KURL::List);
	void slotRemoveItems(QVaueList<Transfer *>);
	void slotSetPriority(QVaueList<Transfer *>, int);
	void slotSetOperation(QVaueList<Transfer *>, enum Operation);
	void slotSetGroup(QVaueList<Transfer *>, const QString &);

private:
		
};

#endif
