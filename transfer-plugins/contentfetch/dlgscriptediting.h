/* This file is part of the KDE project

   Copyright (C) 2008 Ningyu Shi <shiningyu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef DLG_SCRIPT_EDITING_H
#define DLG_SCRIPT_EDITING_H

#include "ui_dlgscriptediting.h"

#include <QStringList>

class DlgScriptEditing : public KDialog
{
    Q_OBJECT

    public:
        DlgScriptEditing(QWidget *p_parent);
        DlgScriptEditing(QWidget *p_parent, const QStringList &script);
        ~DlgScriptEditing();
        void init();
        QString scriptPath() const;
        QString scriptUrlRegexp() const;
        QString scriptDescription() const;
    private slots:
        void slotChangeText();
    private:
        Ui::DlgScriptEditing ui;
};

#endif // DLG_SCRIPT_EDITING_H
