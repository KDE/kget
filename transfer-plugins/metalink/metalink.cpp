/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2007 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include <QDomElement>

#include <kiconloader.h>
#include <klocale.h>
#include <kdebug.h>

#include "metalink.h"

metalink::metalink(TransferGroup * parent, TransferFactory * factory,
                         Scheduler * scheduler, const KUrl & source, const KUrl & dest,
                         const QDomElement * e)
    : Transfer(parent, factory, scheduler, source, dest, e)
{

}

void metalink::start()
{
    kDebug(5001) << "metalink::start" << endl;
}

void metalink::stop()
{
    kDebug(5001) << "Stop" << endl;
}

int metalink::elapsedTime() const
{
    return -1; //TODO
}

int metalink::remainingTime() const
{
    return -1; //TODO
}

bool metalink::isResumable() const
{
    return true;
}

void metalink::load(QDomElement e)
{
    Transfer::load(e);
}

void metalink::save(QDomElement e)
{
    Transfer::save(e);
}


//NOTE: INTERNAL METHODS

#include "metalink.moc"
