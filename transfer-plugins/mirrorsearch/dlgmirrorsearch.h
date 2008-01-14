/* This file is part of the KDE project

   Copyright (C) 2008 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef DLGMIRRORSEARCH_H
#define DLGMIRRORSEARCH_H

#include "ui_dlgengineediting.h"
#include "ui_dlgmirrorsearch.h"

class DlgEngineEditing : public KDialog
{
    Q_OBJECT

public:
    DlgEngineEditing(QWidget *parent = 0);
    ~DlgEngineEditing();

    QString engineName() const;
    QString engineUrl() const;
private slots:
    void slotChangeText();

private:
    Ui::DlgEngineEditing ui;
};

class DlgSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    DlgSettingsWidget(KDialog *parent = 0);
    ~DlgSettingsWidget();

private slots:
    void slotNewEngine();
    void slotRemoveEngine();
    void slotSave();
    void init();
    void enableButtonApply();

private:
    void addSearchEngineItem(const QString &name, const QString &url);

    void loadSearchEnginesSettings();
    void saveSearchEnginesSettings();

    Ui::DlgMirrorSearch ui;
    KDialog *m_parent;
};

#endif // DLGMULTISEGKIO_H
