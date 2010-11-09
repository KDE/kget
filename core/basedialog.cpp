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

#include "basedialog.h"

KGetBaseDialog::KGetBaseDialog(QWidget *parent, Qt::WFlags flags)
  : KDialog(parent, flags)
{
}

void KGetBaseDialog::slotButtonClicked(int button)
{
    if ((button == KDialog::User1) || (button == KDialog::User2) || (button == KDialog::User3)) {
        done(button);
    } else {
        KDialog::slotButtonClicked(button);
    }
}
