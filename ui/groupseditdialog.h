/* This file is part of the KDE project

   Copyright (C) 2006 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#ifndef GROUPSEDITDIALOG_H
#define GROUPSEDITDIALOG_H

#include <KDialog>

class QListView;

class GroupsEditDialog : public KDialog
{
    Q_OBJECT

    public:
        GroupsEditDialog(QWidget *parent = 0);

    private slots:
        void slotAddGroup();
        void slotDeleteGroup();

    private:
        QListView *groupList;
};

#endif

