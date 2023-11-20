/* This file is part of the KDE project

   Copyright (C) 2006 Manolo Valdes <nolis71cu@gmail.com>
   Copyright (C) 2009 Matthias Fuchs <mat69@gmx.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef DLGMETALINK_H
#define DLGMETALINK_H

#include "ui_dlgmetalink.h"

#include <KCModule>

class DlgSettingsWidget : public KCModule
{
    Q_OBJECT
public:
    explicit DlgSettingsWidget(QObject *parent = nullptr, const KPluginMetaData &args = {});
    ~DlgSettingsWidget() override;

public Q_SLOTS:
    void save() override;
    void load() override;

private:
    Ui::DlgMetalink ui;
};

#endif // DLGMETALINK_H
