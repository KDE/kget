/* This file is part of the KDE project

   Copyright (C) 2008 Lukas Appelhans <l.appelhans@gmx.de> 

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#include "pluginselector.h"

#include "core/kget.h"

#include <KServiceTypeTrader>
#include <KService>
#include <KLocale>
#include <KPluginInfo>
#include <KSharedConfig>
#include <KDialog>

PluginSelector::PluginSelector(KDialog * parent)
  : KPluginSelector(parent)
{
    KService::List offers = KServiceTypeTrader::self()->query("KGet/Plugin");

    addPlugins(KPluginInfo::fromServices(offers), KPluginSelector::ReadConfigFile, i18n("Plugins"), "Service", KGlobal::config());

    load();

    connect(parent, SIGNAL(accepted()), SLOT(saveState()));
    connect(parent, SIGNAL(rejected()), SLOT(loadDefaults()));
}

PluginSelector::~PluginSelector()
{
}

void PluginSelector::saveState()
{
    save();
    KGet::loadPlugins();
}

void PluginSelector::loadDefaults()
{
    defaults();
}

#include "pluginselector.moc"
