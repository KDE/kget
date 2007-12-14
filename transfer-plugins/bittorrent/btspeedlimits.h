/* This file is part of the KDE project

   Copyright (C) 2007 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef BTSPEEDLIMITS_H
#define BTSPEEDLIMITS_H

#include "ui_btspeedlimits.h"

#include "bttransferhandler.h"

#include <KDialog>

class BTSpeedLimits : public KDialog, public Ui::BTSpeedLimits
{
    Q_OBJECT
    public:
        BTSpeedLimits(BTTransferHandler * handler, QWidget *parent = 0);

    private slots:
        void setSpeedLimits();

    private:
        BTTransferHandler * m_handler;
};

#endif
