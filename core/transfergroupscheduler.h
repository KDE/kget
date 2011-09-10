/* This file is part of the KDE project

   Copyright (C) 2008 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef TRANSFERGROUPSCHEDULER_H
#define TRANSFERGROUPSCHEDULER_H

#include "core/scheduler.h"

/**
 * @brief TransferGroupScheduler class: what handle all the transfers in kget.
 *
 * This class handles all transfers of KGet, it is a modified Scheduler
 *
 */

class TransferGroupScheduler : public Scheduler
{
    Q_OBJECT
    public:
        explicit TransferGroupScheduler(QObject *parent = 0);
        ~TransferGroupScheduler();

        /**
         * Calculates the whole SpeedLimits
         */
        void calculateSpeedLimits();

        /**
         * Calculates the DownloadLimits
         */
        void calculateDownloadLimit();

        /**
         * Calculates the DownloadLimits
         */
        void calculateUploadLimit();

        /**
         * Sets a download limit to the scheduler
         * @param limit the download limit
         */
        void setDownloadLimit(int limit);

        /**
         * @return the transfergroupschedulers download limit
         */
        int downloadLimit() const {return m_downloadLimit;}

        /**
         * Sets a upload limit to the scheduler
         * @param limit the upload limit
         */
        void setUploadLimit(int limit);

        /**
         * @return the transfergroupschedulers upload limit
         */
        int uploadLimit() const {return m_uploadLimit;}

    private:
        int m_downloadLimit;
        int m_uploadLimit;
};

#endif
 
