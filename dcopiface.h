/* This file is part of the KDE project

   Copyright (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef DCOP_IFACE_H
#define DCOP_IFACE_H

#include <dcopobject.h>
#include <kurl.h>

class DCOPIface : public DCOPObject
{
    K_DCOP

protected:
    DCOPIface( DCOPCString objId ) : DCOPObject( objId ) {}

k_dcop:
    /**
     * Add new transfers via DCOP.
     *
     * @param src The urls to download
     * @param destDir The destination direction or QString() if unspecified
     */
    virtual ASYNC addTransfers( const KUrl::List& src, const QString& destDir = QString() ) = 0;

    virtual bool isDropTargetVisible() const = 0;

    virtual void setDropTargetVisible( bool setVisible ) = 0;

    virtual void setOfflineMode( bool offline ) = 0;

    virtual bool isOfflineMode() const = 0;
};

#endif
