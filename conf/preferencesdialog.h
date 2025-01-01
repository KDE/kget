/* This file is part of the KDE project
   Copyright (C) 2004 - 2007 KGet Developers <kget@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include "ui_dlgadvanced.h"

#include <KConfigDialog>

class QWidget;
class KConfigSkeleton;

class PreferencesDialog : public KConfigDialog
{
    Q_OBJECT

public:
    PreferencesDialog(QWidget *parent, KConfigSkeleton *config);

Q_SIGNALS:
    void resetDefaults();

private Q_SLOTS:
    void slotToggleAfterFinishAction(Qt::CheckState state);
    void slotToggleAutomaticDeletion(Qt::CheckState state);
    void slotCheckExpiryValue();
    void disableApplyButton();
    void enableApplyButton();
    void updateWidgetsDefault() override;

private:
    Ui::DlgAdvanced dlgAdv;
};

#endif
