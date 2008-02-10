/* This file is part of the KDE project

   Copyright (C) 2008 Urs Wolfer <uwolfer @ kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "dlgwebinterface.h"

#include "settings.h"

DlgWebinterface::DlgWebinterface(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);

    readConfig();
}

DlgWebinterface::~DlgWebinterface()
{
}

void DlgWebinterface::readConfig()
{
}

void DlgWebinterface::saveSettings()
{
}

#include "dlgwebinterface.moc"
