/* This file is part of the KDE project

   Copyright (C) 2006 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#ifndef MTDETAILSWIDGET_H
#define MTDETAILSWIDGET_H

#include <QWidget>

#include "mtdetailswidgetfrm.h"

class MTDetailsWidget : public QWidget
{
    Q_OBJECT
    public:
        MTDetailsWidget();

    private:
        Ui::MTDetails frm;

    private slots:
        void slotSetThreads();
};

#endif // MTDETAILSWIDGET_H
