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

#ifndef TRANSFERGRAPH_H
#define TRANSFERGRAPH_H

#include <QObject>
#include <QVariant>
#include <QSizeF>
#include <QColor>

class QPainter;
class QRect;

static const QColor MyColors[] = {
    QColor(Qt::yellow),
    QColor(Qt::blue),
    QColor(Qt::darkMagenta),
    QColor(Qt::red),
    QColor(Qt::green),
    QColor(Qt::darkYellow)
};
static const int TRANSFER_MARGIN = 30;
static const int TRANSFER_APPLET_WIDTH = 380;
static const int TRANSFER_LINE_HEIGHT = 32;
static const int HORIZONTAL_MARGIN = 5;
static const int VERTICAL_MARGIN = 50;

class TransferGraph : public QObject
{
    Q_OBJECT
public:
    TransferGraph(QObject *parent = 0);

    virtual void setTransfers(const QVariantMap &percents);

    QVariantMap transfers() const
    {
        return m_transfers;
    };

    virtual void paint(QPainter *p, const QRect &contentsRect) 
    {
        Q_UNUSED(p)
        Q_UNUSED(contentsRect)
    };
    virtual QSizeF contentSizeHint();

protected:
    virtual void drawTitle(QPainter *p, const QRect &contentsRect);

    QVariantMap m_transfers;
};

#endif
