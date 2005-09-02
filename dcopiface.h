/****************************************************************************
** $Id$
**
** Copyright (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>
**
****************************************************************************/

#ifndef DCOP_IFACE_H
#define DCOP_IFACE_H

#include <dcopobject.h>
#include <kurl.h>

#include <QByteArray>

class DCOPIface : public DCOPObject
{
    K_DCOP

protected:
    DCOPIface( QByteArray objId ) : DCOPObject( objId ) {}

k_dcop:
    /**
     * Add new transfers via DCOP.
     *
     * @param src The urls to download
     * @param destDir The destination direction or QString::null if unspecified
     */
    virtual ASYNC addTransfers( const KURL::List& src, const QString& destDir = QString::null ) = 0;

    virtual bool isDropTargetVisible() const = 0;

    virtual void setDropTargetVisible( bool setVisible ) = 0;

    virtual void setOfflineMode( bool offline ) = 0;

    virtual bool isOfflineMode() const = 0;
};

#endif
