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

#include <QGraphicsProxyWidget>
#include <QList>
#include <QMap>

class QProgressBar;
class QGridLayout;

class KGetPanelBar::Private : public QGraphicsProxyWidget
{
Q_OBJECT
public:
    Private(QGraphicsWidget *parent = 0);
    ~Private();

    void setTransfers(const QVariantMap &transfers);
    QGridLayout *dialogLayout() {
        return m_dialogLayout;
    };

private:
    void showActiveTransfer(const QString &key, const QVariantList &attributes);
    void clear();

private:
    QProgressBar *m_bar;
    QGridLayout *m_dialogLayout;

    QVariantMap m_transfers;
    QMap <QString, int> m_activeTransfers;
    QMap <int, QProgressBar *> m_activeBars;
    QList <QWidget *> m_widgets;
};

#endif
