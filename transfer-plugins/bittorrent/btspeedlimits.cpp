/* This file is part of the KDE project

   Copyright (C) 2007 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#include "btspeedlimits.h"

#include <KDebug>

BTSpeedLimits::BTSpeedLimits(BTTransferHandler * handler, QWidget * parent)
  : KDialog(parent),
    m_handler(handler)
{
    connect(this, SIGNAL(accepted()), SLOT(setSpeedLimits()));
}

void BTSpeedLimits::setSpeedLimits()
{
    m_handler->setTrafficLimits(dlBox->value(), ulBox->value());
}