/***************************************************************************
                          getfilejob.h  -  description
                             -------------------
    begin                : Wed May 8 2002
    copyright            : (C) 2002 by Patrick Charbonnier
    email                : pch@freeshell.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef GETFILEJOB_H
#define GETFILEJOB_H

#include <kio/jobclasses.h>


namespace KIO
{

class GetFileJob:public FileCopyJob
{
public:
    GetFileJob(const KURL & m_src, const KURL & m_dest);
    ~GetFileJob();
    bool getCanResume() const;
};
}

#endif
