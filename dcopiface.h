/****************************************************************************
** $Id$
**
** Copyright (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>
**
****************************************************************************/

#ifndef KGET_IFACE_H
#define KGET_IFACE_H

#include <dcopobject.h>
#include <kurl.h>

class DCOPIface : public DCOPObject
{
    K_DCOP

protected:
    DCOPIface( QCString objId ) : DCOPObject( objId ) {}

k_dcop:
    /**
     * @param src The urls to download
     * @parem destDir The destination direction or QString::null if unspecified
     */
    virtual ASYNC addTransfers( const KURL::List& src, const QString& destDir = QString::null ) = 0;

    virtual bool isDropTargetVisible() const = 0;

    virtual void setDropTargetVisible( bool setVisible ) = 0;

    virtual void setOfflineMode( bool offline ) = 0;

    virtual bool isOfflineMode() const = 0;
};

#endif // KGET_IFACE_H
