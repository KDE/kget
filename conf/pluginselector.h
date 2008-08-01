/* This file is part of the KDE project

   Copyright (C) 2008 Lukas Appelhans <l.appelhans@gmx.de> 

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#ifndef PLUGINSELECTOR_H
#define PLUGINSELECTOR_H

#include <KPluginSelector>

class KDialog;

class PluginSelector : public KPluginSelector
{
    Q_OBJECT
    public:
        PluginSelector(KDialog * parent);
        ~PluginSelector();

    private slots:
        void saveState();
        void loadDefaults();
};

#endif
