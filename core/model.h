/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>

   Based on:
       kmainwidget.{h,cpp} Copyright (C) 2002 by Patrick Charbonnier
       that was based On Caitoo v.0.7.3 (c) 1998 - 2000, Matej Koss

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#ifndef _Model_H
#define _Model_H


#include <qvaluelist.h>
#include <qstring.h>

#include "scheduler.h"

class Transfer;
class TransferGroup;
class TransferHandler;
class ModelObserver;
class KURL;

/**
 * This is our Model class. This is where the user's transfers and searches are
 * stored and organized.
 * Use this class from the views to add or remove transfers or searches 
 * In order to organize the transfers inside categories we have a TransferGroup
 * class. By definition, a transfer must always belong to a TransferGroup. If we 
 * don't want it to be displayed by the gui inside a specific group, we will put 
 * it in the group named "Not grouped" (better name?).
 **/

class Model
{
    public:
        static Model& self()
        {
            static Model m;
            return m;
        }

        /**
         * Adds a new observer of the Model. See observer.h for more info about it.
         *
         * observer: the new observer
         */
        static void addObserver(ModelObserver * observer);

        /**
         * Removes an observer of the Model. See observer.h for more info about it.
         *
         * observer: the observer to remove
         */
        static void delObserver(ModelObserver * observer);


        /**
         * ---------  Model retrieval  ---------
         */
        //static const QValueList<TGroupHandler *> transferGroups();

        /**
         * ---------  Group managing functions ---------
         */

        /**
         * Adds a new group to the Model.
         *
         * groupName: the name of the new group
         */
        static void addGroup(const QString& groupName);

        /**
         * Removes a group from the Model.
         *
         * groupName: the name of the group to be deleted
         */
        static void delGroup(const QString& groupName);

        /**
         * ---------  Transfer managing functions ---------
         */

        /**
         * Adds a new transfer to the Model
         *
         * src: the url to be downloaded. ### WARNING! THIS IS TEMPORARY! ### We must
         *      provide a common way to add generic transfers to the Model, that
         *      must be able to handle every kind of transfers (including torrent or
         *      filesharing protocols).
         * destDir: the destination directory
         * groupName: the name of the group the new transfer will belong to.
         */
        static void addTransfer(KURL src, const QString& destDir, 
                                const QString& groupName = QString("Not grouped"));

        /**
         * Removes a transfer from the Model
         *
         * transfer: the transfer to be removed.
         */
        static void delTransfer(TransferHandler * transfer);

        /**
         * Moves a transfer to a new group
         *
         * transfer: the transfer to be moved
         * groupName: the name of the new transfer's group
         */
        static void moveTransfer(TransferHandler * transfer, const QString& groupName);

    private:
        Model();

        /**
         * IDEAS ON HOW REORGANIZING THESE FUNCTIONS: new functions should be:
         * - KURL urlInputDialog();
         * - KURL destInputDialog();
         * - bool isValidSource(KURL source);
         * - bool isValidDest(KURL dest);
         *
         *
         *
         *
         */


        /**
         * Deletes the given file, if possible, and returns true.
         * If the given url is a directory or if it is not local it returns
         * false and shows a warning message.
         *
         * url: the file to delete
         */
        static bool safeDeleteFile( const KURL& url );

        static QMap<QString, TransferGroup *> m_transferGroups;
        static QValueList<ModelObserver *> m_observers;

        static Scheduler m_scheduler;
};


#endif
