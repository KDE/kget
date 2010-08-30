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

#ifndef INTEGRATIONPREFERENCES 
#define INTEGRATIONPREFERENCES 

#include <QtGui/QWidget>

#include "ui_dlgintegration.h"

class AutoPasteModel;
class KConfigDialog;

class IntegrationPreferences : public QWidget
{
    Q_OBJECT

    public:
        explicit IntegrationPreferences(KConfigDialog *parent, Qt::WindowFlags f = 0);
        ~IntegrationPreferences();

    private slots:
        void slotUpdateButtons();
        void slotAddItem();
        void slotRemoveItem();
        void slotIncreasePriority();
        void slotDecreasePriority();

    signals:
        /**
         * Emitted whenever something changes
         */
        void changed();

    private:
        Ui::DlgIntegration ui;
        AutoPasteModel *m_model;
};

#endif

