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

#ifndef GENERALWIDGET_H
#define GENERALWIDGET_H

#include <QWidget>

#include "ui_generalwidget.h"

namespace KGetMetalink
{
    class Metalink;
}

class GeneralWidget : public QWidget
{
    Q_OBJECT

    public:
        GeneralWidget(QWidget *parent = 0);

        void load(const KGetMetalink::Metalink &metalink) const;
        void save(KGetMetalink::Metalink *metalink);

    private slots:
        /**
         * To correctly enable checkboxes
         */
        void slotPublishedEnabled(bool enabled);

        /**
         * To correctly enable checkboxes
         */
        void slotUpdatedEnabled(bool enabled);

    private:
        Ui::GeneralWidget ui;
};

#endif
