/***************************************************************************
                                Errors.h
                             -------------------
    Revision 				: $Id$
    begin						: Tue Jan 29 2002
    copyright				: (C) 2002 by Patrick Charbonnier
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
#include <kdebug.h>

#define DEFAULT_FTP_PORT 21

#define ERR_CANNOT_OPEN_FOR_READING   1
#define ERR_CANNOT_OPEN_FOR_WRITING   2
#define ERR_CANNOT_LAUNCH_PROCESS   3
#define ERR_INTERNAL   4
#define ERR_MALFORMED_URL   5
#define ERR_UNSUPPORTED_PROTOCOL   6
#define ERR_NO_SOURCE_PROTOCOL   7
#define ERR_UNSUPPORTED_ACTION   8
#define ERR_IS_DIRECTORY   9
#define ERR_IS_FILE   10
#define ERR_DOES_NOT_EXIST   11
#define ERR_FILE_ALREADY_EXIST   12
#define ERR_DIR_ALREADY_EXIST   13
#define ERR_UNKNOWN_HOST   14
#define ERR_ACCESS_DENIED   15
#define ERR_WRITE_ACCESS_DENIED   16
#define ERR_CANNOT_ENTER_DIRECTORY   17
#define ERR_PROTOCOL_IS_NOT_A_FILESYSTEM   18
#define ERR_CYCLIC_LINK   19
#define ERR_USER_CANCELED   20
#define ERR_CYCLIC_COPY   21
#define ERR_COULD_NOT_CREATE_SOCKET   22
#define ERR_COULD_NOT_CONNECT   23
#define ERR_CONNECTION_BROKEN   24
#define ERR_NOT_FILTER_PROTOCOL   25
#define ERR_COULD_NOT_MOUNT   26
#define ERR_COULD_NOT_UNMOUNT   27
#define ERR_COULD_NOT_READ   28
#define ERR_COULD_NOT_WRITE   29
#define ERR_COULD_NOT_BIND   30
#define ERR_COULD_NOT_LISTEN   31
#define ERR_COULD_NOT_ACCEPT   32
#define ERR_COULD_NOT_LOGIN   33
#define ERR_COULD_NOT_STAT   34
#define ERR_COULD_NOT_CLOSEDIR   35
#define ERR_COULD_NOT_MKDIR   37
#define ERR_COULD_NOT_RMDIR   38
#define ERR_CANNOT_RESUME   39
#define ERR_CANNOT_RENAME   40
#define ERR_CANNOT_CHMOD   41
#define ERR_CANNOT_DELETE   42
#define ERR_SLAVE_DIED   43
#define ERR_OUT_OF_MEMORY   44
#define ERR_UNKNOWN_PROXY_HOST   45
#define ERR_COULD_NOT_AUTHENTICATE   46
#define ERR_ABORTED   47
#define ERR_INTERNAL_SERVER   48
#define ERR_SERVER_TIMEOUT   49
#define ERR_SERVICE_NOT_AVAILABLE   50
#define ERR_UNKNOWN   51
#define ERR_UNKNOWN_INTERRUPT   53
#define ERR_CANNOT_DELETE_ORIGINAL   54
#define ERR_CANNOT_DELETE_PARTIAL   55
#define ERR_CANNOT_RENAME_ORIGINAL   56
#define ERR_CANNOT_RENAME_PARTIAL   57
#define ERR_NEED_PASSWD   58
#define ERR_CANNOT_SYMLINK   59
#define ERR_NO_CONTENT   60
#define ERR_DISK_FULL   61
#define ERR_IDENTICAL_FILES   62
#define SUCCESS 0
#define FAIL -1
#define READ 2
