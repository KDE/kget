/* This file is part of the KDE project
   
   Copyright (C) 2004 Enrico Ros <eros.kde@email.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/


#ifndef _CONNECTION_H
#define _CONNECTION_H

#include <QObject>

#include "globals.h"

/**
  * This class 
  *
  * @short ...
  **/

class Connection : public QObject
{
    public:
	Connection( QObject * parent, const char * name = "" );
	~Connection();

	bool isOnline();

    private slots:
	void slotCheckConnections();

};

#endif

