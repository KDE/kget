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

#ifndef PIECHARTWIDGET_H
#define PIECHARTWIDGET_H

#include <QGraphicsWidget>

class PieChartWidget : public QGraphicsWidget
{
Q_OBJECT

public:
    PieChartWidget(QGraphicsWidget *parent = 0);
    ~PieChartWidget();

    void addData(const QString &name, double length);
    void addData(const QString &name, double length, double activeLength);
    void addData(const QString &name, double length, double activeLength, bool active);
    void removeData(const QString &name);
    void clear();
    void updateView();

    QSizeF sizeHint(Qt::SizeHint, const QSizeF&) const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, 
                        QWidget *widget);

signals:
    // emitted when the geometry of the applet who contains the widget needs to be updated (new data to display ...)
    void geometryChanged();

private:
    class PrivateData; // each piece of the pie
    class Private;
    Private * const d;
};
#endif

