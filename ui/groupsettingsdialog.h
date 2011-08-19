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
#include "../core/basedialog.h"

#include <KDialog>

#ifdef HAVE_NEPOMUK
namespace Nepomuk {
    class TagWidget;
}
#endif

class TransferGroupHandler;

class GroupSettingsDialog : public KGetSaveSizeDialog
{
    Q_OBJECT
    public:
        GroupSettingsDialog(QWidget *parent, TransferGroupHandler *group);
        ~GroupSettingsDialog();

        QSize sizeHint() const;

    private slots:
        void save();

    private:
#ifdef HAVE_NEPOMUK
        void updateUsedTagsLineEdit();
#endif //HAVE_NEPOMUK

    private:
        TransferGroupHandler* m_group;
        Ui::GroupSettingsDialog ui;

#ifdef HAVE_NEPOMUK
        Nepomuk::TagWidget *m_tagWidget;
#endif //HAVE_NEPOMUK
};

#endif
