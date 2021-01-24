/***************************************************************************
*   Copyright (C) 2010 Matthias Fuchs <mat69@gmx.net>                     *
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

#ifndef KGET_BASE_DIALOG
#define KGET_BASE_DIALOG

#include "kget_export.h"
#include <QDialog>


/**
 * Subclass to make sure that the size of the dialog is
 * automatically stored and restored
 */
class KGET_EXPORT KGetSaveSizeDialog : public QDialog
{
    Q_OBJECT

    public:
        /**
         * Restores the dialog to the size saved for name
         */
        explicit KGetSaveSizeDialog(const QByteArray &name, QWidget *parent = nullptr, Qt::WindowFlags flags = {});
        ~KGetSaveSizeDialog() override;
        
    private:
        const QByteArray m_name;
};

#endif
