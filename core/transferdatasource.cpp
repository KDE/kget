/* This file is part of the KDE project

   Copyright (C) 2008 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
 
#include "transferdatasource.h"

#include <kdebug.h>

TransferDataSource::TransferDataSource(const KUrl &srcUrl, QObject *parent)
  : QObject(parent),
    m_sourceUrl(srcUrl),
    m_speed(0),
    m_supposedSize(0)
{
    kDebug(5001) ;
}

TransferDataSource::~TransferDataSource()
{
    kDebug(5001) ;
}

#include "transferdatasource.moc"
