/* This file is part of the KDE project
   
   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/


#ifndef _TRANSFER_KIO_H
#define _TRANSFER_KIO_H

#include "transfer.h"

#include <kio/job.h>

/**
 * This transfer uses the KIO class to download files
 */
 
class TransferKio : public Transfer
{
Q_OBJECT

    friend class TransferList;

public:
    TransferKio(Scheduler * _scheduler, const KURL & _src, const KURL & _dest);
    TransferKio(Scheduler * _scheduler, QDomNode * n);
    
public slots:
    virtual bool slotResume();
    virtual void slotStop();
    virtual void slotRetransfer();
    virtual void slotRemove();
    
    virtual void slotSetSpeed(int speed);
    virtual void slotSetDelay(int seconds);
    virtual void slotSetSegmented(int nSegments);
    
private:
    void createJob();

    KIO::FileCopyJob * copyjob;
        
private slots:
    void slotResult( KIO::Job *job );
    void slotInfoMessage( KIO::Job *job, const QString & msg );
    void slotConnected( KIO::Job *job );
    void slotPercent( KIO::Job *job, unsigned long percent );
    void slotTotalSize( KIO::Job *job, KIO::filesize_t size );
    void slotProcessedSize( KIO::Job *job, KIO::filesize_t size );
    void slotSpeed( KIO::Job *job, unsigned long bytes_per_second );
};

#endif
