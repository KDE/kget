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

#include <QMap>

#include <plasma/plasma_export.h>
#include <plasma/widgets/widget.h>

class PieChartWidget : public Plasma::Widget
{
Q_OBJECT

public:
    PieChartWidget(Widget *parent);
    ~PieChartWidget();

    void addData(QString name, int length);
    void addData(QString name, int length, int activeLength);
    void addData(QString name, int length, int activeLength, bool active);
    void removeData(const QString &name);
    void clear();
    void updateView();

    QSizeF sizeHint() const;
    void paintWidget(QPainter *painter, const QStyleOptionGraphicsItem *option, 
                        QWidget *widget);

signals:
    // emited when the geometry of the applet who contains the widget needs to be updated (new data to display ...)
    void geometryChanged();

private:
    class PrivateData; // each piece of the pie
    class Private;
    Private * const d;
};
#endif
