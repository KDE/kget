/* This file is part of the KDE project

   Copyright (C) 2008 Ningyu Shi <shiningyu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef DLG_CONTENT_FETCH_SETTING_H
#define DLG_CONTENT_FETCH_SETTING_H

#include "ui_dlgcontentfetchsettingwidget.h"

class DlgContentFetchSettingWidget : public QWidget
{
    Q_OBJECT

public:
    DlgContentFetchSettingWidget(KDialog *p_parent = 0);
    ~DlgContentFetchSettingWidget();

private slots:
    void slotNewScript();
    void slotEditScript();
    void slotRemoveScript();
    void slotSave();

private:
    void addScriptItem(const QString &path, const QString &regexp,
		       const QString &description);
    void loadContentFetchSetting();
    void saveContentFetchSetting();

    Ui::DlgContentFetchSettingWidget ui;
    KDialog *m_p_parent;
    bool m_changed;
};

#endif // DLG_CONTENT_FETCH_SETTING_H
