/* This file is part of the KDE project
   
   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/


#include <kio/job.h>
#include <kdebug.h>

#include "transferKio.h"
#include "core/globals.h"

#include "transferKio.moc"

TransferKio::TransferKio(Scheduler * _scheduler, const KURL & _src, const KURL & _dest)
    : Transfer(_scheduler, _src, _dest),
      copyjob(0)
{


}

TransferKio::TransferKio(Scheduler * _scheduler, QDomNode * n)
    : Transfer(_scheduler, n),
      copyjob(0)
{
    Transfer::read(n);
}

bool TransferKio::slotResume()
{
    if(!copyjob)
    {
        if(!createJob())
            return false;
    }
        
    tInfo.status = St_Trying;
    setTransferChange(Tc_Status);    

    emit transferChanged(this);
    
    return true;
}

void TransferKio::slotStop()
{
    if(copyjob)
    {
        copyjob->kill(true);
        copyjob=0;
    }
    
    kdDebug() << "slotStop 4" << endl;
    tInfo.status = St_Stopped;
    tInfo.speed = 0;
    setTransferChange(Tc_Status);
    setTransferChange(Tc_Speed);    
    
    emit transferChanged(this);
}

void TransferKio::slotRetransfer()
{
    
}

void TransferKio::slotRemove()
{

}
    
void TransferKio::slotSetSpeed(int speed)
{

}

void TransferKio::slotSetSegmented(int nSegments)
{

}


//NOTE: INTERNAL METHODS

bool TransferKio::createJob()
{
    if(!copyjob)
    {
        if( !(copyjob = KIO::file_copy(tInfo.src, tInfo.dest, -1, false, false, false)) )
        {
            kdDebug() << "###############################################" << endl;
            return false;
        }
                                          
        connect(copyjob, SIGNAL(result(KIO::Job *)), 
                this, SLOT(slotResult(KIO::Job *)));
        connect(copyjob, SIGNAL(infoMessage(KIO::Job *, const QString &)), 
                this, SLOT(slotInfoMessage(KIO::Job *, const QString &)));
        connect(copyjob, SIGNAL(connected(KIO::Job *)), 
                this, SLOT(slotConnected(KIO::Job *)));
        connect(copyjob, SIGNAL(percent(KIO::Job *, unsigned long)), 
                this, SLOT(slotPercent(KIO::Job *, unsigned long)));
        connect(copyjob, SIGNAL(totalSize(KIO::Job *, KIO::filesize_t)), 
                this, SLOT(slotTotalSize(KIO::Job *, KIO::filesize_t)));
        connect(copyjob, SIGNAL(processedSize(KIO::Job *, KIO::filesize_t)), 
                this, SLOT(slotProcessedSize(KIO::Job *, KIO::filesize_t)));
        connect(copyjob, SIGNAL(speed(KIO::Job *, unsigned long)), 
                this, SLOT(slotSpeed(KIO::Job *, unsigned long)));
    }
    return true;
}

//enum TransferStatus { ST_TRYING, ST_RUNNING, ST_STOPPED, ST_FINISHED };

void TransferKio::slotResult( KIO::Job *job )
{
    kdDebug() << "slotResult" << endl;
    switch (job->error())
    {
        case 0:
            tInfo.status = St_Finished;
            tInfo.percent = 100;
            tInfo.speed = 0;
            tInfo.processedSize = tInfo.totalSize;
            setTransferChange(Tc_Percent);
            setTransferChange(Tc_Speed);
            break;
        default:
            //There has been an error
            tInfo.status = St_Aborted;
            kdDebug() << "--  E R R O R  --" << endl;
            copyjob=0;
            break;
    }
    setTransferChange(Tc_Status);
    emit transferChanged(this);
}

void TransferKio::slotInfoMessage( KIO::Job *job, const QString & msg )
{
    tInfo.log.push_back(new QString(msg));
}

void TransferKio::slotConnected( KIO::Job *job )
{
//     kdDebug() << "CONNECTED" <<endl;
    
    tInfo.status = St_Running;
    setTransferChange(Tc_Status);
    emit transferChanged(this);
}

void TransferKio::slotPercent( KIO::Job *job, unsigned long percent )
{
//     kdDebug() << "slotPercent" << endl; 
    
    if(tInfo.status != St_Running)
        slotConnected(job);
        
    tInfo.percent = percent;
    setTransferChange(Tc_Percent);
    emit transferChanged(this);
}

void TransferKio::slotTotalSize( KIO::Job *job, KIO::filesize_t size )
{
//     kdDebug() << "slotTotalSize" << endl; 
    
    if(tInfo.status != St_Running)
        slotConnected(job);
    
    tInfo.totalSize = size;
    setTransferChange(Tc_TotalSize);
    emit transferChanged(this);
}

void TransferKio::slotProcessedSize( KIO::Job *job, KIO::filesize_t size )
{
    kdDebug() << "slotProcessedSize" << endl; 
    
    if(tInfo.status != St_Running)
        slotConnected(job);
    
    tInfo.processedSize = size;
    setTransferChange(Tc_ProcessedSize);
    emit transferChanged(this);
}

void TransferKio::slotSpeed( KIO::Job *job, unsigned long bytes_per_second )
{
//     kdDebug() << "slotSpeed" << endl;
    
    if(tInfo.status != St_Running)
        slotConnected(job);
    
    tInfo.speed = bytes_per_second;
    setTransferChange(Tc_Speed);
    emit transferChanged(this);
}

