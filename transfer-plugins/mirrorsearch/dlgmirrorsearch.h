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

#include <KCModule>
#include <QDialog>

class DlgEngineEditing : public QDialog
{
    Q_OBJECT

public:
    DlgEngineEditing(QWidget *parent = nullptr);
    ~DlgEngineEditing() override;

    QString engineName() const;
    QString engineUrl() const;
private Q_SLOTS:
    void slotChangeText();

private:
    Ui::DlgEngineEditing ui;
    QPushButton *okButton;
};

class DlgSettingsWidget : public KCModule
{
    Q_OBJECT

public:
    explicit DlgSettingsWidget(QObject *parent = nullptr, const KPluginMetaData &args = {});
    ~DlgSettingsWidget() override;

public Q_SLOTS:
    void save() override;
    void load() override;

private Q_SLOTS:
    void slotNewEngine();
    void slotRemoveEngine();

private:
    void addSearchEngineItem(const QString &name, const QString &url);

    void loadSearchEnginesSettings();
    void saveSearchEnginesSettings();

    Ui::DlgMirrorSearch ui;
    QDialog *m_parent;
};

#endif // DLGMULTISEGKIO_H
