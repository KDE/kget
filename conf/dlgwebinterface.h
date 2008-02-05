/* This file is part of the KDE project

   Copyright (C) 2008 Urs Wolfer <uwolfer @ kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef DLGWEBINTERFACE_H
#define DLGWEBINTERFACE_H

#include <QWidget>

#include "ui_dlgwebinterface.h"

class DlgWebinterface : public QWidget, public Ui::DlgWebinterface
{
    Q_OBJECT

public:
    DlgWebinterface(QWidget *parent = 0);
    ~DlgWebinterface();

private Q_SLOTS:
    void readConfig();
    void saveSettings();
    void changePasswordButtonClicked();
};

#endif
