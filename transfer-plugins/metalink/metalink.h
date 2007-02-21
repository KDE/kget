/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/


#ifndef METALINK_H
#define METALINK_H

#include <kio/job.h>

#include "core/transfer.h"

 
class metalink : public QObject, public Transfer
{
    Q_OBJECT

    public:
        metalink(TransferGroup * parent, TransferFactory * factory,
                    Scheduler * scheduler, const KUrl & src, const KUrl & dest,
                    const QDomElement * e = 0);

    public slots:
        // --- Job virtual functions ---
        void start();
        void stop();

        int elapsedTime() const;
        int remainingTime() const;
        bool isResumable() const;

        void save(QDomElement e);

    protected:
        void load(QDomElement e);
};

#endif
