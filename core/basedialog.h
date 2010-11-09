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

#include "../kget_export.h"
#include <KDialog>

/**
 * Normal KDialog class, only that if User1, User2 or User3 is pressed
 * a corresponding return code is set
 */
class KGET_EXPORT KGetBaseDialog : public KDialog
{
    Q_OBJECT

    public:
        explicit KGetBaseDialog(QWidget *parent = 0, Qt::WFlags flags = 0);

    protected slots:
        virtual void slotButtonClicked(int button);
};

#endif
