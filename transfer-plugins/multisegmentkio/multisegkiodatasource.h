/* This file is part of the KDE project

   Copyright (C) 2008 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
 
#ifndef KGET_MULTISEGKIODATASOURCE_H
#define KGET_MULTISEGKIODATASOURCE_H

#include "core/transferdatasource.h"

#include <kio/job.h>

class SegmentFactory;

class MultiSegKioDataSource : public TransferDataSource
{
    public:
        MultiSegKioDataSource();
        ~MultiSegKioDataSource();

        void start();
	void stop();
        void addSegment(const KUrl &srcUrl, const KIO::fileoffset_t offset, const KIO::fileoffset_t bytes);

    private:
        SegmentFactory * m_SegFactory;
};

#endif
