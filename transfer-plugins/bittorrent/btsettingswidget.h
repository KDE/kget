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

#include <KCModule>

class BTSettingsWidget : public KCModule, public Ui::BTSettingsWidget
{
    Q_OBJECT
public:
    BTSettingsWidget(QObject *parent = nullptr, const KPluginMetaData &args = {});

public Q_SLOTS:
    void load() override;
    void save() override;
    void defaults() override;
};

#endif
