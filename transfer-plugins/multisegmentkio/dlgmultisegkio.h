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

class DlgSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    DlgSettingsWidget(KDialog *parent = 0);
    ~DlgSettingsWidget();

private slots:
    void slotSetUseSearchEngines(bool b);
    void slotSave();
    void init();
    void enableButtonApply();

private:
    Ui::DlgMultiSeg ui;
    KDialog *m_parent;
};

#endif // DLGMULTISEGKIO_H
