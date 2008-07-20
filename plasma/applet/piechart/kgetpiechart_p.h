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
#ifndef KGETPIECHART_P_H
#define KGETPIECHART_P_H

#include <math.h>

#include <QGraphicsWidget>
#include <QMap>

#include <KColorCollection>

class QPainter;
class QRect;
class QBrush;
class QStyleOptionGraphicsItem;

class KGetPieChart::Private : public QGraphicsWidget
{
Q_OBJECT
public:
    Private(QGraphicsWidget *parent = 0);
    ~Private();

    void setTransfers(const QVariantMap &transfers);
    void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);
    void update();

private:
    // draw a portion of the pie chart and returns his end angle
    int paintPieData(QPainter *p, const QRect &rect, int angle, int percent, const QBrush &brush);
     // Draw the graph legend with the names of the data
    void drawLegend(const QString &name, QPainter *p, const QStyleOptionGraphicsItem *option, const QColor &color, int count);

    inline double roundNumber(float number)
    {
        double intPart;
        if (modf(number, &intPart) > 0.5) {
            return intPart + 1;
        }
        else {
            return intPart;
        }
    };

private:
    QMap <QString, PrivateData> m_data;
    QVariantMap m_transfers;
    KColorCollection m_colors;

    int m_totalSize;
    bool m_needsRepaint;
};

class KGetPieChart::PrivateData
{
public:
    PrivateData()
    {};

    QString name;
    bool isActive;
    double length;
    double activeLength;
};

#endif
