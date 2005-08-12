/* This file is part of the KDE project
   Copyright (C) 2004 KGet Developers < >

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#ifndef _PREFERENCESDIALOG_H
#define _PREFERENCESDIALOG_H

#include <kconfigdialog.h>

// single config pages
#include "dlgappearance.h"
#include "dlgnetwork.h"
#include "dlgdirectories.h"
#include "dlgadvanced.h"

using namespace Ui;

class QWidget;
class KConfigSkeleton;

class PreferencesDialog : public KConfigDialog
{
    public:
        PreferencesDialog( QWidget * parent, KConfigSkeleton * config );

    protected:
    // 	void updateSettings(); // Called when OK/Apply is pressed.
    // 	void updateWidgets(); // Called upon construction or when Reset is pressed
    // 	void updateWidgetsDefault(); // Called when Defaults button is pressed
    // 	bool hasChanged(); // In order to correctly disable/enable Apply button
    // 	bool isDefault(); //  In order to correctly disable/enable Defaults button

    private:
        QWidget * appearance;
        QWidget * network;
        QWidget * directories;
        QWidget * advanced;
};

#endif
