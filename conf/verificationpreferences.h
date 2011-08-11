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

#ifndef VERIFICATIONPREFERENCES_H
#define VERIFICATIONPREFERENCES_H

#include <QtGui/QWidget>

#include "ui_verificationpreferences.h"

class KConfigDialog;

class VerificationPreferences : public QWidget
{
    Q_OBJECT

    public:
        explicit VerificationPreferences(KConfigDialog *parent, Qt::WindowFlags f = 0);

    signals:
        /**
         * Emitted when the mirrors change
         */
        void changed();

    private slots:
        void slotDefaultClicked();
        void slotAccpeted();
        void slotRejected();

    private:
        Ui::VerificationPreferences ui;
        QStringList m_tempKeyServers;
};

#endif
