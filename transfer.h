/***************************************************************************
*                                transfer.h
*                             -------------------
*
*    Revision     : $Id$
*    begin        : Tue Jan 29 2002
*    copyright    : (C) 2002 by Patrick Charbonnier
*                 : Based On Caitoo v.0.7.3 (c) 1998 - 2000, Matej Koss
*    email        : pch@freeshell.org
*
****************************************************************************/

/***************************************************************************
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 ***************************************************************************/


#ifndef _Transfer_h
#define _Transfer_h

#include <qlistview.h>
#include <qdatetime.h>
#include <qguardedptr.h>
#include <qmap.h>

#include <kurl.h>
#include <kio/jobclasses.h>
#include <ftpsearch.h>
//#include "slave.h"
#include "slaveftp.h"
#include "slavehttp.h"


class QTimer;


class KSimpleConfig;
class KAction;
class KRadioAction;

class KPingPool;

class DlgIndividual;
class TransferList;


class Transfer:public QObject, public QListViewItem
{

Q_OBJECT public:

        enum TransferMode { MD_QUEUED, MD_DELAYED, MD_SCHEDULED, MD_NONE };
        /*
          enum TransferStatus
          {
            ST_CHECKING_RESUME,ST_CHECKING_SIZE,  ST_RESUMING, ST_GETTING,
        		ST_RUNNING, ST_NOT_RUNNING,  ST_FINISHED,    ST_STOPPED,  ST_ABORTED, ST_PAUSED                       };
          */
        enum TransferStatus {
                ST_RUNNING, ST_FINISHED, ST_STOPPED
        };

        //emitted to inform KMainWidget that something is changed...like a task is done.,..
        enum TransferOperation {
                OP_CAN_RESUME_CHECKED, OP_SIZE_CHECKED, OP_FINISHED,
                OP_FINISHED_KEEP,
                OP_CANCELED, OP_ERROR, OP_PAUSED, OP_REMOVED,
                OP_RESUMED, OP_QUEUED, OP_SCHEDULED, OP_DELAYED, OP_ABORTED
        };



        Transfer(TransferList * view, const KURL & _src, const KURL & _dest);
        Transfer(TransferList * view, Transfer * after, const KURL & _src, const KURL & _dest);
        ~Transfer();
        Slave *m_pSlave;
        void copy(Transfer *);

        bool read(KSimpleConfig * config, int id);
        void write(KSimpleConfig * config, int id);
        void logMessage(const QString & message);
        // getter methods
        KURL getSrc()
        {
                return src;
        }
        KURL getDest()
        {
                return dest;
        }
        QDateTime getStartTime()
        {
                return startTime;
        }
        unsigned long getTotalSize()
        {
                return totalSize;
        }
        unsigned long getProcessedSize()
        {
                return processedSize;
        }
        int getPercent()
        {
                return percent;
        }
        QString getUser()
        {
                return src.user();
        }

        void setStatus(TransferStatus _status);

        QString getPass()
        {
                return src.pass();
        }
        void setNameAndPasswork(const QString & _user, const QString & _pass)
        {
                src.setPass(_pass);
                src.setUser(_user);
                m_pSlave->m_src.setPass(_pass);
                m_pSlave->m_src.setUser(_user);
                updateAll();

        }


        int getTotalFiles()
        {
                return totalFiles;
        }
        int getProcessedFiles()
        {
                return processedFiles;
        }
        int getSpeed()
        {
                return speed;
        }
        QTime getRemainingTime()
        {
                return remainingTime;
        }
        int getStatus();
        int getMode()
        {
                return mode;
        }

        void setMode(TransferMode _mode)
        {
                mode = _mode;
        }


        // setter methods
        //  void setSrc (const KURL & _src);

        //  void setDest (const KURL & _dest);

        void setStartTime(QDateTime _startTime);
        void setSpeed(unsigned long _speed);
        /*   gia dichiarata
          void setMode (TransferMode _mode)
          {
            mode = _mode;
          }
          */
        // update methods
        void updateAll();
        bool updateStatus(int counter);

        void showIndividual();

        /** No descriptions */
        void Test(void);
        /** No descriptions */

        /** No descriptions */
        void ResumeStatus();
        /** No descriptions */

        // actions
        KAction *m_paResume, *m_paPause, *m_paDelete, *m_paRestart;
        KRadioAction *m_paQueue, *m_paTimer, *m_paDelay;
        /**  */


public slots:
        // operation methods
        void slotSearch();
        void slotResume();
        void slotPause();
        void slotPauseOffline();
        void slotRestart();
        void slotRemove();
        void slotUpdateActions();

        void slotQueue();
        void slotFinished();
        void slotSpeed(unsigned long);
        void slotTotalSize(unsigned long bytes);
        void slotProcessedSize(unsigned long);
        void slotCanResume();

        /** No descriptions */
        void slotSchedule();
        /** No descriptions */
        void slotDelay();
        /** No descriptions */
        void slotExecPause();
        /** No descriptions */
        //  void slotExecAbort();
        /** No descriptions */

        void slotExecRemove();
        /** No descriptions */

        void slotExecDelay();
        /** No descriptions */
        void slotExecSchedule();
        /** No descriptions */
        void slotStartTime(const QDateTime &);
        /** No descriptions */
        void slotExecResume();
        /** No descriptions */
        void SlotExecLoginInfo();
        /** No descriptions */
        void slotExecAbort(const QString &);
        /** No descriptions */
        void slotExecCanResume(bool _bCanResume);

signals:
        void statusChanged(Transfer *, int _operation);
        void log(uint, const QString &, const QString &);
        void searchStarted();
        void found(QString url);
        void pingSpeed(QString host, float speed);

protected slots:
        // job signals
        void slotCanceled(KIO::Job *);


        void slotTotalFiles(unsigned long);

        void slotProcessedFiles(unsigned long);


        void slotCopying(const KURL &, const KURL &);
        void slotRenaming(KIO::Job *, const KURL &, const KURL &);


        //  void slotSearchResult (KIO::Job *);

        // void slotFoundItem (const QString &, const QString &, const QString &);
        // void slotPingSpeed (QString, float);

        //  void slotSearchTimeout ();




        /** No descriptions */
protected:

private:
        void setupFields();
        /** No descriptions */

        KURL src;
        KURL dest;

        uint id;


        // KIO::FileCopyJob * copyjob;
        //KIO::Job * copyjob;
        //  KIO::StatJob * statjob;

        // search stuff
        //  KSearch::FtpJob * searchJob;
        //  float highestSpeed;
        //  QString fastestHost;
        //  bool b_saturated;

        // typedef QMap < QString, float >HostMap;
        //  HostMap hosts;
        //  QStringList sources;

        // schedule time
        QDateTime startTime;

        unsigned long totalSize;
        unsigned long processedSize;
        int percent;
        int totalFiles;
        int processedFiles;

        int speed;
        QTime remainingTime;

        TransferStatus status;
        TransferMode mode;

        // how many times have we retried already
        int retryCount;

        bool canResume;

        TransferList *view;

        //  QTimer *searchTimer;

        // individual download window
        DlgIndividual *dlgIndividual;

        static uint idcount;
        // static KSearch::FtpSiteManager * searchManager;
        // static KPingPool *pinger;
        /**  */
        Slave::SlaveCommand m_NextCmd;


        /**  */
        //FTP_RESULT result;
}
;


#endif				// _Transfer_h
