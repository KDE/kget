/***************************************************************************
*                                slave.cpp
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

/*
 
Function ReadableTimeOut : DroneUtil  Copyright (c) 1999-2000 Theodore C. Belding
 													 University of Michigan Center for the Study of Complex Systems
 													<mailto:Ted.Belding@umich.edu>
 													<http://www-personal.umich.edu/~streak/>
 
*/


#include "slave.h"
#include "common.h"

#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>		/* needed by HPUX for memset in FD_ZERO macro */


Slave::Slave(Transfer * _parent, const KURL & _src, const KURL & _dest)
{


        mDebug << ">>>>Entering" << endl;

        m_src = _src;
        m_dest = _dest;
        m_parent = _parent;
        m_CanResume = 0;


        mDebug << ">>>>Leaving" << endl;


}

Slave::~Slave()
{}

/** No descriptions
void Slave::setDest(const KURL & _dest){
m_dest=_dest;
}
*/
/** No descriptions
void Slave::setSrc(const KURL & _src){
m_src=_src;
}
*/
void
Slave::Op(SlaveCommand _cmd)
{

        m_break = true;
        m_cmd = _cmd;
        switch (_cmd) {
        case PAUSE:
        case RESTART:
        case ABORT:
        case DELAY:
        case SCHEDULE:
        case REMOVE:
                m_status = SLV_STOPPING;
                break;
        case CHECK_SIZE:
        case CHECK_RESUME:
        case RETR:
                m_break = false;
                m_status = SLV_RUNNING;
                break;

        }
}

/** No descriptions */
inline void Slave::PostMessage(SlaveResult _event, unsigned long _data)
{

        SlaveEvent *e1 = new SlaveEvent(m_parent, _event, _data);
        postEvent(kapp->mainWidget(), (QEvent *) e1);

}

inline void Slave::PostMessage(SlaveResult _event, const QString & _msg)
{

        SlaveEvent *e1 = new SlaveEvent(m_parent, _event, _msg);
        postEvent(kapp->mainWidget(), (QEvent *) e1);
        mDebug << "Msg:" << "_msg = " << _msg << endl;

}

void Slave::error(int _error, QString _msg)
{
        mDebug << "Error:" << _error << "_msg = " << _msg << endl;
        SlaveResult event;
        event = SLV_ERR;
        if (_msg == QString::null)
                _msg = "Unknow Error";
        //_msg="Unknow Error";
        m_status = SLV_ABORTING;
        switch (_error) {
        case ERR_CANNOT_OPEN_FOR_READING:
	  break;
        case ERR_CANNOT_OPEN_FOR_WRITING:
          _msg = "Cannot Open for Writing:"  +_msg;
	  break;
        case ERR_CANNOT_LAUNCH_PROCESS:
        case ERR_INTERNAL:
        case ERR_MALFORMED_URL:
        case ERR_UNSUPPORTED_PROTOCOL:
        case ERR_NO_SOURCE_PROTOCOL:
        case ERR_UNSUPPORTED_ACTION:
        case ERR_IS_DIRECTORY:
        case ERR_IS_FILE:
                break;
        case ERR_DOES_NOT_EXIST:
                _msg = m_src.path() + "not Found";
                break;
        case ERR_FILE_ALREADY_EXIST:
        case ERR_DIR_ALREADY_EXIST:
                break;
        case ERR_UNKNOWN_HOST:
                _msg = "Error unknow host  " + _msg;
                break;
        case ERR_ACCESS_DENIED:
        case ERR_WRITE_ACCESS_DENIED:
        case ERR_CANNOT_ENTER_DIRECTORY:
        case ERR_PROTOCOL_IS_NOT_A_FILESYSTEM:
        case ERR_CYCLIC_LINK:
        case ERR_USER_CANCELED:
        case ERR_CYCLIC_COPY:
        case ERR_COULD_NOT_CREATE_SOCKET:
                break;
        case ERR_COULD_NOT_CONNECT:
                _msg = "Error could not connect to  " + _msg;
                break;
        case ERR_CONNECTION_BROKEN:
        case ERR_NOT_FILTER_PROTOCOL:
        case ERR_COULD_NOT_MOUNT:
        case ERR_COULD_NOT_UNMOUNT:
                break;
        case ERR_COULD_NOT_READ:
                _msg = "Reading from socket failed" + _msg;
                ;
                break;
        case ERR_COULD_NOT_WRITE:
        case ERR_COULD_NOT_BIND:
        case ERR_COULD_NOT_LISTEN:
        case ERR_COULD_NOT_ACCEPT:
                break;
        case ERR_COULD_NOT_LOGIN:
                event = SLV_ERR_COULD_NOT_LOGIN;
                _msg = "Could not Login into " + m_src.host();
                break;
        case ERR_COULD_NOT_STAT:
        case ERR_COULD_NOT_CLOSEDIR:
        case ERR_COULD_NOT_MKDIR:
        case ERR_COULD_NOT_RMDIR:
        case ERR_CANNOT_RESUME:
        case ERR_CANNOT_RENAME:
        case ERR_CANNOT_CHMOD:
        case ERR_CANNOT_DELETE:
        case ERR_SLAVE_DIED:
        case ERR_OUT_OF_MEMORY:
        case ERR_UNKNOWN_PROXY_HOST:
        case ERR_COULD_NOT_AUTHENTICATE:
        case ERR_ABORTED:
        case ERR_INTERNAL_SERVER:
                break;
        case ERR_SERVER_TIMEOUT:
                event = SLV_ERR_SERVER_TIMEOUT;
                _msg = "Server Time Out";
                break;
        case ERR_SERVICE_NOT_AVAILABLE:
        case ERR_UNKNOWN:
        case ERR_UNKNOWN_INTERRUPT:
        case ERR_CANNOT_DELETE_ORIGINAL:
        case ERR_CANNOT_DELETE_PARTIAL:
        case ERR_CANNOT_RENAME_ORIGINAL:
        case ERR_CANNOT_RENAME_PARTIAL:
        case ERR_NEED_PASSWD:
        case ERR_CANNOT_SYMLINK:
        case ERR_NO_CONTENT:
        case ERR_DISK_FULL:
        case ERR_IDENTICAL_FILES:
                break;
        }
        PostMessage(event, _msg);
}


void Slave::run()
{
        mDebug << ">>>>Entering" << endl;
        mDebug << endl;
        mDebug << endl;
        mDebug << "Command: " << m_cmd << " src= " << m_src.
        url() << " Dest= " << m_dest.url() << endl;
        mDebug << endl;
        mDebug << endl;

        openConnection();

        if (m_status == SLV_RUNNING) {
                switch (m_cmd) {
                case CHECK_RESUME:
                        CanResume();
                        break;
                case CHECK_SIZE:
                        GetRemoteSize();
                        break;
                case RETR:
                        PostMessage(SLV_RESUMED);
                        retr();
                        break;
                }

        }


        SlaveResult event;


        unsigned long data = m_size;

        if (m_status == SLV_FINISHING) {
                //ok we finish the  job..
                event = SLV_FINISHED;
                closeConnection();

        } else if (m_status == SLV_ABORTING && !m_break) {
                //already emitted signal...
                //event=SLV_ABORTED;
                mDebug << ">>>>Leaving with m_status= " << m_status << endl;
                closeConnection();
                return;

        } else {
                switch (m_cmd) {
                case DELAY:
                        event = SLV_DELAYED;
                        closeConnection();
                        break;
                case PAUSE:
                        event = SLV_PAUSED;
                        closeConnection();
                        break;
                case SCHEDULE:
                        event = SLV_SCHEDULED;
                        closeConnection();
                        break;
                case CHECK_RESUME:
                        event = SLV_CHECKED_RESUME;
                        data = m_CanResume;
                        break;
                case CHECK_SIZE:
                        event = SLV_CHECKED_SIZE;
                        break;
                case ABORT:
                        event = SLV_ABORTED;
                        break;
                case REMOVE:
                        event = SLV_REMOVED;
                        break;
                default:
                        event = SLV_UNKNOW_EVENT;
                        closeConnection();
                        assert(0);
                }



        }

        PostMessage(event, data);

        mDebug << ">>>>Leaving with m_status= " << m_status << endl;



}


/** No descriptions */
bool Slave::CheckLocalOffset()
{


        mDebug << ">>>>Entering" << endl;

        // now must check if file exists...

        //QFile f(m_strFileName);
        mDebug << "the destination file " << m_dest.path() << endl;

        QFile f(m_dest.path());

        if (f.open(IO_ReadOnly)) {	// file opened successfully
                m_offset = f.size();
                mDebug << " the filesize is " << m_offset << endl;

        } else {
                mDebug << " destination file doesn't exists " << m_offset << endl;
                m_offset = 0;
        }

        mDebug << ">>>>Leaving" << endl;
        return true;
}


/* Description: This routine waits at most a specified interval for a
 file descriptor to become readable.
 
 wait up to sec seconds for file descriptor fd to become readable
* if sec is 0, returns immediately
* returns:
* > 0 if descriptor is readable
* < 0 if error (errno set by select())
* == 0 if timeout
*/
int Slave::ReadableTimeOut(int fd, int sec)
{
        fd_set rset;
        struct timeval tv;

        FD_ZERO(&rset);
        FD_SET(fd, &rset);

        tv.tv_sec = sec;
        tv.tv_usec = 0;

        return (::select(fd + 1, &rset, NULL, NULL, &tv));

}
