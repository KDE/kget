/* This file is part of the KDE project

   Copyright (C) 2006 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef DLGMULTISEGKIO_H
#define DLGMULTISEGKIO_H

#include "ui_dlgmultisegkio.h"

#include <KCModule>

class DlgSettingsWidget : public KCModule
{
    Q_OBJECT
public:
    explicit DlgSettingsWidget(QWidget * parent = nullptr, const QVariantList &args = QVariantList());
    ~DlgSettingsWidget() override;

public Q_SLOTS:
    void save() override;
    void load() override;

private:
    Ui::DlgMultiSeg ui;
};

#endif // DLGMULTISEGKIO_H
