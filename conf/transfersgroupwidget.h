/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2007 Urs Wolfer <uwolfer @ kde.org>
   Copyright (C) 2007 Javier Goday <jgoday @ gmail.com>
   Copyright (C) 2010 Matthias Fuchs <mat69@gmx.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef TRANSFERS_GROUP_WIDGET_H
#define TRANSFERS_GROUP_WIDGET_H

#include "ui_dlggroups.h"

class TransfersGroupWidget : public QWidget
{
    Q_OBJECT
    public:
        TransfersGroupWidget(QWidget *parent = 0);

    private slots:
        void slotSelectionChanged();

    private:
        Ui::DlgGroups ui;
};

#endif
