#ifndef _TRANSFER_H
#define _TRANSFER_H

#include <qobject.h>
#include <qvaluevector.h>
#include <qdatetime.h>

#include <kdebug.h>
#include <kurl.h>

class QDomElement;
class QDomNode;
class QDomDocument;

class TransferList;
class Scheduler;
class ViewInterface;

class Transfer : public QObject
{
Q_OBJECT
    friend class TransferList;

    public:
       
    enum TransferStatus  { St_Trying,
                           St_Running,
                           St_Delayed,
                           St_Stopped,
                           St_Aborted,
                           St_Finished
                         };  

    typedef int TransferProgress;                         
                         
    enum ProgressChange  { Pc_None          = 0x00000000,
                           Pc_Priority      = 0x00000001,
                           Pc_Status        = 0x00000002,
                           Pc_CanResume     = 0x00000004,
                           Pc_TotalSize     = 0x00000008,
                           Pc_ProcessedSize = 0x00000010,
                           Pc_Percent       = 0x00000020,
                           Pc_Speed         = 0x00000040,
                           Pc_Log           = 0x00000080 
                         };
    
    struct Info
    {
        int priority;
        TransferStatus status;
        QValueList <QString *> log;
        
        KURL src;
        KURL dest;
        
        unsigned long totalSize;
        unsigned long processedSize;
        int percent;
    
        int speed;
            
        QString group;
        QDateTime startTime;
        QTime remainingTime;
        int delayTime;
        
        bool canResume;
        
        /**
        * This field specifies whether the transfer can perform segmented 
        * downloading or not.
        */
        bool canSegment;
    };

  
    protected:
    
    Info info;
    Scheduler * sched;
    QValueVector<TransferProgress> progressChanges;
        
    /**
     * These functions can be reimplemented if necessary
     */
    
    virtual bool read(QDomDocument * doc, QDomNode * n);
    virtual void write(QDomDocument * doc, QDomNode * n);

    inline void setProgressChange(ProgressChange);
    
        
    public slots:
    
    /**
     * These slots _MUST_ be reimplemented
     */
    
    virtual bool slotResume() {kdDebug() << "Transfer::slotResume" << endl;}
    virtual void slotStop() {}
    virtual void slotRetransfer() {}
    virtual void slotRemove() {}
    
    virtual void slotSetSpeed(int speed) {}
    virtual void slotSetDelay(int seconds) {}
    virtual void slotSetSegmented(int nSegments) {}
   
    signals:
    
    void statusChanged(Transfer *, Transfer::TransferStatus message);    
    void progressChanged(Transfer *, Transfer::ProgressChange message);

    public:
        
    Transfer(Scheduler * _scheduler, const KURL & _src, const KURL & _dest);
    Transfer(Scheduler * _scheduler, QDomDocument * doc, QDomNode * e);
    
    const Info& getInfo() const;
    
    TransferProgress getProgressFlags(ViewInterface *);
    void resetProgressFlags(ViewInterface *);
       
    void setPriority(int p);
    void setGroup(const QString& group);
    
    inline bool operator<(const Transfer& t2) const
        {return info.priority < t2.info.priority;}
    
    inline bool operator<=(const Transfer& t2) const
        {return info.priority <= t2.info.priority;}

    inline bool operator>(const Transfer& t2) const
        {return info.priority > t2.info.priority;}
    
    inline bool operator>=(const Transfer& t2) const
        {return info.priority >= t2.info.priority;}
        
    inline bool operator==(const Transfer& t2) const
        {return info.priority == t2.info.priority;}
            
        
    void about() const;
};

#endif
