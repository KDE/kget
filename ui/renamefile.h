/***************************************************************************
*   Copyright (C) 2009 Matthias Fuchs <mat69@gmx.net>                     *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
***************************************************************************/

#ifndef RENAMEFILE_H
#define RENAMEFILE_H

#include <KDialog>

#include "ui_renamefile.h"

class FileModel;

class RenameFile : public KDialog
{
    Q_OBJECT

    public:
        RenameFile(FileModel *model, const QModelIndex &index, QWidget *parent = 0, Qt::WFlags flags = 0);

        void setOriginalName(const QString &originalName);

    private slots:
        void updateButton();
        void rename();

    private:
        FileModel *m_model;
        QModelIndex m_index;
        Ui::RenameFile ui;
        KUrl m_dest;
};

#endif
