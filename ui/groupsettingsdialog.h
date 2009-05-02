/* This file is part of the KDE project

   Copyright (C) 2008 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/ 
#ifndef GROUPSETTINGSDIALOG_H
#define GROUPSETTINGSDIALOG_H

#include "ui_groupsettingsdialog.h"

#include <KDialog>

#ifdef HAVE_NEPOMUK
    #include "ui_grouptagssettingsdialog.h"
    class KMenu;
#endif
class TransferGroupHandler;

class GroupSettingsDialog : public KDialog
{
    Q_OBJECT
    public:
        GroupSettingsDialog(QWidget *parent, TransferGroupHandler *group);
        ~GroupSettingsDialog();

    private slots:
#ifdef HAVE_NEPOMUK
        void modelClicked(QModelIndex index);
        void tagClicked(const QString &tag);
        void addCurrentTag();
        void removeCurrentTag();
        void addNewTag();
        void textChanged(const QString &text);
#endif //HAVE_NEPOMUK
        void save();

    private:
#ifdef HAVE_NEPOMUK
        void updateUsedTagsLineEdit();
#endif //HAVE_NEPOMUK

    private:
        TransferGroupHandler* m_group;
        Ui::GroupSettingsDialog ui;

#ifdef HAVE_NEPOMUK
        Ui::GroupTagsSettingsDialog tagsUi;
        QStringList m_tags;
        QString m_currentTag;
        QStringListModel *m_usedTagsModel;
        KMenu *m_popup;
        QAction *m_removeAction;
        QAction *m_addAction;
#endif //HAVE_NEPOMUK
};

#endif
