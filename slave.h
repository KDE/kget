/***************************************************************************
                                slave.h
                             -------------------
    Revision 				: $Id$
    begin						: Tue Jan 29 2002
    copyright				: (C) 2002 by Patrick Charbonnier
									: Based On Caitoo v.0.7.3 (c) 1998 - 2000, Matej Koss
    email						: pch@freeshell.og
 ***************************************************************************/

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


#ifndef SLAVE_H
#define SLAVE_H

#include <qthread.h>
#include <kurl.h>
#include <kapp.h>
#include <qfile.h>
#include "slaveevent.h"
#include "Errors.h"
#include "common.h"
#include <assert.h>
#include <klocale.h>
/**
  *@author Patrick Charbonnier
  */
class Transfer;

class Slave:public QThread
{
public:
        enum SlaveCommand {
                CHECK_RESUME, CHECK_SIZE, RETR, PAUSE, RESTART, ABORT, DELAY,
                SCHEDULE,
                REMOVE
        };

        enum SlaveResult {
                SLV_PROGRESS_SPEED, SLV_RESUMED, SLV_PROGRESS_SIZE,
                SLV_CHECKED_SIZE, SLV_CHECKED_RESUME, SLV_PAUSED,
                SLV_ABORTED, SLV_DELAYED, SLV_SCHEDULED,
                SLV_FINISHED, SLV_INFO, SLV_UNKNOW_EVENT,
                SLV_REMOVED, SLV_ERR, SLV_ERR_COULD_NOT_LOGIN,
                SLV_ERR_UNKNOWN_HOST, SLV_ERR_COULD_NOT_CONNECT,
                SLV_ERR_SERVER_TIMEOUT
        };

        enum SlaveStatus {

                SLV_RUNNING, SLV_STOPPING, SLV_FINISHING, SLV_ABORTING
        };

public:
        Slave(Transfer * _parent, const KURL & _src, const KURL & _dest);
        ~Slave();
        /** No descriptions */
        //  virtual void setSrc(const KURL & _dest);
        /** No descriptions */
        //  virtual void setDest(const KURL & _dest);
        /** No descriptions */
        void Op(SlaveCommand _cmd);
        void error(int _error, QString _msg = QString::null);
        /** No descriptions */
        void PostMessage(SlaveResult _event, unsigned long _data = 0L);
        void PostMessage(SlaveResult _event, const QString & _msg);


private:			// Private attributes




protected:
        SlaveStatus m_status;
        Transfer *m_parent;
        bool m_break;


        /** contain the last command */
        SlaveCommand m_cmd;


public:			// Public attributes
        KURL m_src;
        KURL m_dest;
public:			// Public attributes
        /**  */
        bool m_CanResume;
        size_t m_size;
        unsigned long m_offset;
        unsigned long m_processed_size;
        /**  */
        unsigned int m_speed;
        /**  */

        virtual void run();

        virtual void openConnection() = 0;
        virtual void CanResume() = 0;
        virtual void GetRemoteSize() = 0;
        virtual void retr() = 0;

        virtual void closeConnection() = 0;
        bool CheckLocalOffset();
        /** No descriptions */
        int ReadableTimeOut(int fd, int sec);


};

#endif
