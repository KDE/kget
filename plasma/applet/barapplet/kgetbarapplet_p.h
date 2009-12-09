/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   Copyright (C) 2007 by Javier Goday <jgoday@gmail.com>                 *
 *   Copyright (C) 2009 by Matthias Fuchs <mat69@gmx.net>                  *
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
#ifndef KGETBARAPPLET_P_H
#define KGETBARAPPLET_P_H

#include <QGraphicsWidget>
#include <QHash>

namespace Plasma
{
    class ScrollWidget;
}

class QGraphicsProxyWidget;
class QProgressBar;

class KGetBarApplet::Private : public QGraphicsWidget
{
    Q_OBJECT

    public:
        Private(QGraphicsWidget *parent = 0);
        ~Private();

    public slots:
        void addTransfers(const QList<OrgKdeKgetTransferInterface*> &transfers);
        void removeTransfers(const QList<OrgKdeKgetTransferInterface*> &transfers);

    private slots:
        void slotUpdateTransfer(int transferChange);

    private:
        struct Item
        {
            QGraphicsProxyWidget *proxy;
            QProgressBar *progressBar;
        };

        Plasma::ScrollWidget *m_scrollWidget;
        QGraphicsWidget *m_containerWidget;
        QGraphicsLinearLayout *m_containerLayout;
        QHash<OrgKdeKgetTransferInterface*, Item*> m_items;
};

#endif
