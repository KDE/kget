/* This file is part of the KDE project
   Copyright (C) 2004 KGet Developers < >

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include "settings.h"

void DlgAdvanced::init()
{
    downloadDT->setDateTime( Settings::timedDownloadDateTime() );
    disconnectDT->setDateTime( Settings::timedDisconnectDateTime() );
    connect( downloadDT, SIGNAL(valueChanged(const QDateTime &)),
	     this, SLOT(slotDownloadDT(const QDateTime &)) );
    connect( disconnectDT, SIGNAL(valueChanged(const QDateTime &)),
	     this, SLOT(slotDisconnectDT(const QDateTime &)) );
}

void DlgAdvanced::slotDownloadDT( const QDateTime &dt )
{
    Settings::setTimedDownloadDateTime( dt );
}

void DlgAdvanced::slotDisconnectDT( const QDateTime &dt )
{
    Settings::setTimedDisconnectDateTime( dt );
}
