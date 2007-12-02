/* This file is part of the KDE project

   Copyright (C) 2006 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef DLGMULTISEGKIO_H
#define DLGMULTISEGKIO_H

#include "ui_dlgengineediting.h"
#include "ui_dlgmultisegkio.h"

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
    void slotSetSegments(int seg);
    void slotSetMinSegSize(int size);
    void slotSetSaveDataSize(int size);
    void slotSetUseSearchEngines(bool b);
    void slotNewEngine();
    void slotRemoveEngine();
    void slotSave();
    void init();

private:
    void addSearchEngineItem(const QString &name, const QString &url);

    void loadSearchEnginesSettings();
    void saveSearchEnginesSettings();

    int m_segments;
    int m_minsegsize;
    int m_savesegsize;
    bool m_searchengines;

    Ui::DlgMultiSeg ui;
};

#endif // DLGMULTISEGKIO_H
