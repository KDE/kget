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
#include "scriptconfigadaptor.h"

#include <QPointer>
#include <kross/core/action.h>


class DlgContentFetchSettingWidget : public QWidget
{
    Q_OBJECT

    public:
        DlgContentFetchSettingWidget(KDialog *p_parent = 0);
        ~DlgContentFetchSettingWidget();
    private slots:
        void slotNewScript();
        void slotEditScript();
        void slotConfigureScript();
        void slotRemoveScript();
        void slotSave();
        void slotAccepted();
        void slotRejected();
        void slotCheckConfigurable(QTreeWidgetItem *p_item,
                                   int column);
        void slotEnableChanged(QTreeWidgetItem* p_item,
                               int column);

    signals:
        void configureScript(QWidget* widget, QObject* configadaptor);
        void configurationAccepted(QWidget* widget, QObject* configadaptor);
    private:
        void addScriptItem(bool enabled, const QString &path, const QString &regexp,
                           const QString &description);
        void loadContentFetchSetting();
        void saveContentFetchSetting();

        Ui::DlgContentFetchSettingWidget ui;
        QPointer<KDialog> m_p_parent;
        bool m_changed;
        QPointer<Kross::Action> m_p_action;
};

class SettingWidgetAdaptor : public QWidget
{
    Q_OBJECT
    public:
        explicit SettingWidgetAdaptor(QWidget* parent = 0) : QWidget(parent) {}
        virtual ~SettingWidgetAdaptor() {}
    public slots:
        void setWidget(QWidget* widget)
        {
            widget->setParent(this);
            // make this widget same size as the user supplied one
            // this will make resize easier from outside.
            resize(widget->size());
        }
};

#endif // DLG_CONTENT_FETCH_SETTING_H
