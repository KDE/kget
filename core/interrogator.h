/* This file is part of the KDE project
   
   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>
  
   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/


#ifndef _INTERROGATOR_H
#define _INTERROGATOR_H

#include <kdebug.h>

/**
 * the TransferInterrogator and GroupInterrogator classes are meant only to
 * be inherited by all those classes that want to interrogate transfers or
 * groups by calling the functions:
 *   
 *   (for the TransferInterrogator class)  
 *   TransferChanges Transfer::changesFlags(const TransferInterrogator *);
 *   void Transfer::resetChangesFlags(const TransferInterrogator *);
 *    
 *   (for the GroupInterrogator class)
 *   GroupChanges Group::changesFlags(GroupInterrogator *);
 *   void GroupresetChangesFlags(GroupInterrogator *);
 *
 */

class TransferInterrogator
{
public:
/*    TransferInterrogator::TransferInterrogator()
    {
        static int newId=-1;
        id = ++newId;
        kdDebug() << "new TransferInterrogator: id = " << id << endl;
    }
    
    int getId() const {return id;} */
    
private:
/*    int id;*/
};

class GroupInterrogator
{
public:
/*    GroupInterrogator::GroupInterrogator()
    {
        static int newId=-1;
        id = ++newId;
        kdDebug() << "new GroupInterrogator: id = " << id << endl;
    }
    
    int getId() const {return id;} */
    
private:
/*    int id;*/
};

#endif
