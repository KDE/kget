/* This file is part of the KDE project

   Copyright (C) 2008 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#ifndef PLUGINSELECTOR_H
#define PLUGINSELECTOR_H

#include <KPluginWidget>

class QDialog;

class PluginSelector : public KPluginWidget
{
    Q_OBJECT
public:
    PluginSelector(QDialog *parent);
    ~PluginSelector() override;

private Q_SLOTS:
    void saveState();
    void loadDefaults();
};

#endif
