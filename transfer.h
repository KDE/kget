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

    typedef int TransferChanges;                         
                         
    enum TransferChange  { Tc_None          = 0x00000000,
                           Tc_Priority      = 0x00000001,
                           Tc_Status        = 0x00000002,
                           Tc_CanResume     = 0x00000004,
                           Tc_TotalSize     = 0x00000008,
                           Tc_ProcessedSize = 0x00000010,
                           Tc_Percent       = 0x00000020,
                           Tc_Speed         = 0x00000040,
                           Tc_Log           = 0x00000080 
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
    QValueVector<TransferChanges> transferChanges;
        
    /**
     * These functions can be reimplemented if necessary
     */
    
    virtual bool read(QDomNode * n);
    virtual void write(QDomNode * n);

    inline void setTransferChange(TransferChange);
        
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
    void transferChanged(Transfer *, Transfer::TransferChanges message);

    public:
        
    Transfer(Scheduler * _scheduler, const KURL & _src, const KURL & _dest);
    Transfer(Scheduler * _scheduler, QDomNode * n);
    
    const Info& getInfo() const;
    
    TransferChanges getChangesFlags(ViewInterface *);
    void resetChangesFlags(ViewInterface *);
       
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
