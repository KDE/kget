/* This file is part of the KDE project

   Copyright (C) 2008 Lukas Appelhans <l.appelhans@gmx.de> 

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#include "pluginselector.h"

#include "core/kget.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>
#include <QDialog>

PluginSelector::PluginSelector(QDialog * parent)
  : KPluginWidget(parent)
{
    addPlugins(KGet::self()->plugins(), i18n("Plugins"));
    setConfig(KSharedConfig::openConfig()->group(QStringLiteral("Plugins")));

    connect(parent, &QDialog::accepted, this, &PluginSelector::saveState);
    connect(parent, &QDialog::rejected, this, &PluginSelector::loadDefaults);
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


