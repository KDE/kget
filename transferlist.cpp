/* This file is part of the KDE project
   
   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include <kstandarddirs.h>
#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <kmessagebox.h>
#include <ksimpleconfig.h>
#include <kio/netaccess.h>
#include <assert.h>

#include <qdom.h>
#include <qfile.h>
#include <qtextstream.h>

#include "transfer.h"
#include "transferKio.h"
#include "transferlist.h"
#include "scheduler.h"

TransferList::TransferList()
    : QValueList<Transfer *>()
{

}

TransferList::TransferList(Transfer * transfer)
    : QValueList<Transfer *>()
{
    if(transfer)
        addTransfer(transfer);
}


TransferList::~TransferList()
{

}

void TransferList::addTransfer(Transfer * transfer, bool toBegin)
{
//    sDebugIn << endl;

    iterator it = begin();
    iterator endList = end();
        
    if(toBegin)
        for (; it!=endList && (**it < *transfer); ++it );
    else
        for (; it!=endList && (**it <= *transfer); ++it );
    
    insert(it, transfer);
    
//    sDebugOut << endl;
}

void TransferList::addTransfers(const TransferList & transfers, bool toBegin)
{
    sDebugIn << endl;

    constIterator it = transfers.begin();
    constIterator endList = transfers.end();
    
    for(; it != endList; ++it)
    {
        addTransfer(*it, toBegin);
    }
    sDebugOut << endl;
}

void TransferList::removeTransfer(Transfer * transfer)
{
    sDebugIn << endl;

    remove(transfer);
    
    sDebugOut << endl;
}

void TransferList::removeTransfers(const TransferList & transfers)
{
    sDebugIn << endl;

    constIterator it = transfers.begin();
    constIterator endList = transfers.end();
    
    for(; it != endList; ++it)
    {
        remove(*it);
    }
    sDebugOut << endl;
}

bool TransferList::contains(Transfer * transfer) const
{
    constIterator it = begin();
    constIterator endList = end();
    
    for(; it != endList; ++it)
    {
        if( (*it) == transfer)
            return true;
    }
    return false;
}

Transfer * TransferList::find(const KURL& _src)
{
    iterator it = begin();
    iterator endList = end();
    
    for(; it != endList; ++it)
    {
        if((*it)->info().src == _src)
            return *it;
    }
    return 0;
}

void TransferList::readTransfers(const QString& filename, Scheduler * scheduler, GroupList * g)
{
    sDebugIn << endl;

    QString tmpFile;
        
    if (KIO::NetAccess::download(filename, tmpFile, 0)) 
    {
        kdDebug()<<"__1__"<<endl;
        QFile file(tmpFile);
        QDomDocument doc;
   
        if ( doc.setContent(&file) ) 
        {
            kdDebug()<<"__2__"<<endl;
            QDomElement root = doc.documentElement();
            
            g->read(&doc);
                
            QDomNodeList nodeList = root.elementsByTagName("Transfer");
            int nItems = nodeList.length();
            
            for(int i=0; i<nItems; i++)
            {
                kdDebug()<<"__1__"<<endl;
                TransferKio * t = new TransferKio(scheduler, &nodeList.item(i));
                addTransfer(t);
            }
        }    
        else
        {
            kdWarning()<<"Error reading the transfers file"<< endl; 
        }        
    }
    else
    {
        kdWarning()<<"Error reading the transfers file"<< endl; 
    }        

    sDebugOut << endl;
}

void TransferList::writeTransfers(const QString& filename, Scheduler * scheduler)
{
    sDebug << ">>>>Entering with file =" << filename << endl;

    QDomDocument doc(QString("KGet")+QString(KGETVERSION));
    QDomElement root = doc.createElement("TransferList");
    doc.appendChild(root);

    iterator it = begin();
    iterator endList = end();
    
    scheduler->getGroups().write(&doc);
    
    for (int id = 0; it != endList; ++it)
        (*it)->write(&root);
    
        
            
    QFile file(filename);
    if ( !file.open( IO_WriteOnly ) )
    {
        kdWarning()<<"Unable to open output file when saving"<< endl;
        return;
    }
    
    QTextStream stream( &file );
    doc.save( stream, 0 );
    file.close();
    
    sDebug << "<<<<Leaving" << endl;
}

void TransferList::about()
{
    kdDebug() << "TRANSFERLIST: " << endl;
    
    iterator it;
    iterator endList = end();
    
    for(it = begin(); it != endList; ++it)
    {
        (*it)->about();
    }
}
