#include <kio/job.h>
#include <kdebug.h>

#include "transferKio.h"
#include "globals.h"

TransferKio::TransferKio(Scheduler * _scheduler, const KURL & _src, const KURL & _dest)
    : Transfer(_scheduler, _src, _dest),
      copyjob(0)
{


}

bool TransferKio::slotResume()
{
    kdDebug() << "TransferKio::slotResume (1)" << endl;
    if(!copyjob)
    {
        kdDebug() << "TransferKio::slotResume (2)" << endl;
        createJob();
    }
        
    kdDebug() << "TransferKio::slotResume (3)" << endl;

    info.status = St_Trying;
    setProgressChange(Pc_Status);    
    kdDebug() << "TransferKio::slotResume (4)" << endl;

    emit statusChanged(this, St_Trying);
    kdDebug() << "TransferKio::slotResume (5)" << endl;
    
    return true;
}

void TransferKio::slotStop()
{
    copyjob->kill(true);
    copyjob=0;
    
    info.status = St_Stopped;
    info.speed = 0;
    setProgressChange(Pc_Status);
    setProgressChange(Pc_Speed);    
    
    emit statusChanged(this, St_Stopped);
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

void TransferKio::slotSetDelay(int seconds)
{

}

void TransferKio::slotSetSegmented(int nSegments)
{

}
    
bool TransferKio::read(/*qdom entry*/)
{

}

void TransferKio::write(/*qdom entry*/)
{

}

//NOTE: INTERNAL METHODS

void TransferKio::createJob()
{
    if(!copyjob)
    {
        copyjob = KIO::file_copy(info.src, info.dest, -1, false, false, false);
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
}

//enum TransferStatus { ST_TRYING, ST_RUNNING, ST_STOPPED, ST_FINISHED };

void TransferKio::slotResult( KIO::Job *job )
{
    switch (job->error())
    {
        case 0:
            info.status = St_Finished;
            info.percent = 100;
            info.speed = 0;
            setProgressChange(Pc_Percent);
            setProgressChange(Pc_Speed);
            break;
        default:
            //There has been an error
            info.status = St_Aborted;
            break;
    }
    setProgressChange(Pc_Status);
    emit progressChanged(this, Pc_Status);
        
    emit statusChanged(this, info.status);
}

void TransferKio::slotInfoMessage( KIO::Job *job, const QString & msg )
{
    info.log.push_back(new QString(msg));
}

/*
    enum TransferStatus  { St_Trying;
                           St_Running;
                           St_Delayed;
                           St_Stopped;
                           St_Finished;
                         };  
    
    enum ProgressChange { Pc_None          = 0x00000000;
                           Pc_CanResume     = 0x00000001;
                           Pc_Connected     = 0x00000002;
                           Pc_TotalSize     = 0x00000004;
                           Pc_ProcessedSize = 0x00000008;
                           Pc_Percent       = 0x00000010;
                           Pc_Speed         = 0x00000011;

*/

/*
    void progressChanged(Transfer *, TransferProgress message);

*/

void TransferKio::slotConnected( KIO::Job *job )
{
    kdDebug() << "CONNECTEDCONNECTEDCONNECTEDCONNECTEDCONNECTEDCONNECTED" <<endl;
    
    info.status = St_Running;
    emit statusChanged(this, St_Running);
    setProgressChange(Pc_Status);
    emit progressChanged(this, Pc_Status);
}

void TransferKio::slotPercent( KIO::Job *job, unsigned long percent )
{
    if(info.status != St_Running)
        slotConnected(job);
        
    info.percent = percent;
    setProgressChange(Pc_Percent);
    emit progressChanged(this, Pc_Percent);
}

void TransferKio::slotTotalSize( KIO::Job *job, KIO::filesize_t size )
{
    if(info.status != St_Running)
        slotConnected(job);
    
    info.totalSize = size;
    setProgressChange(Pc_TotalSize);
    emit progressChanged(this, Pc_TotalSize);
}

void TransferKio::slotProcessedSize( KIO::Job *job, KIO::filesize_t size )
{
    if(info.status != St_Running)
        slotConnected(job);
    
    info.processedSize = size;
    setProgressChange(Pc_ProcessedSize);
    emit progressChanged(this, Pc_ProcessedSize);
}

void TransferKio::slotSpeed( KIO::Job *job, unsigned long bytes_per_second )
{
    if(info.status != St_Running)
        slotConnected(job);
    
    info.speed = bytes_per_second;
    setProgressChange(Pc_Speed);
    emit progressChanged(this, Pc_Speed);
}

