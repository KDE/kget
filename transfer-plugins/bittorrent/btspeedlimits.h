/* This file is part of the KDE project

   Copyright (C) 2007 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef BTSPEEDLIMITS_H
#define BTSPEEDLIMITS_H

#include "bttransferhandler.h"

#include <KDialog>

class QSpinBox;
class QDoubleSpinBox;

class BTSpeedLimits : public KDialog
{
    Q_OBJECT
    public:
        explicit BTSpeedLimits(BTTransferHandler * handler, QWidget *parent = 0);

    signals:
        void aboutToClose();

    private slots:
        void setSpeedLimitsAndClose();
        void onlyClose();

    private:
        BTTransferHandler * m_handler;
        QSpinBox *m_dlBox;
        QSpinBox *m_ulBox;
        QDoubleSpinBox *m_shareRatioSpin;
};

#endif
