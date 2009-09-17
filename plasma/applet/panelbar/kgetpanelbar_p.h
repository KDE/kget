/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *
 *   Copyright (C) 2007 by Javier Goday <jgoday@gmail.com>
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
#ifndef KGETPANELBAR_P_H
#define KGETPANELBAR_P_H

#include <plasma/dialog.h>
#include <QList>
#include <QMap>
#include "transfer_interface.h"

class QProgressBar;
class QGridLayout;

class KGetPanelBar::Private : public Plasma::Dialog
{
Q_OBJECT
public:
    Private(QWidget * parent = 0);
    ~Private();
    
    QGridLayout *dialogLayout() {
        return m_dialogLayout;
    };

public slots:
    void transfersAdded(const QList<OrgKdeKgetTransferInterface*> &transfers);
    void transfersRemoved(const QList<OrgKdeKgetTransferInterface*> &transfers);
    void update();
    void clear();

signals:
    void progressBarChanged(int value);

private:
    QProgressBar *m_bar;
    QGridLayout *m_dialogLayout;
    QWidget *m_widget;

    QMap<OrgKdeKgetTransferInterface*, QProgressBar*> m_transfers;
};

#endif
