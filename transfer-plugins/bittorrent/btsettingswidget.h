/* This file is part of the KDE project

   Copyright (C) 2007 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef BTSETTINGSWIDGET_H
#define BTSETTINGSWIDGET_H

#include <ui_btsettingswidget.h>

#include <QWidget>

class KDialog;

class BTSettingsWidget : public QWidget, public Ui::BTSettingsWidget
{
    Q_OBJECT
    public:
        BTSettingsWidget(KDialog * parent);

    private slots:
        void dialogAccepted();
        void setDefault();
        void enableButtonApply();

    private:
        KDialog * m_parent;
};

#endif

