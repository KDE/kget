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

#ifndef LINEGRAPHWIDGET_H
#define LINEGRAPHWIDGET_H

#include <QMap>

#include <QGraphicsWidget>

class LineGraphWidget : public QGraphicsWidget
{
Q_OBJECT

public:
    LineGraphWidget(QGraphicsWidget *parent);
    ~LineGraphWidget();

    void addData(const QString &key, int data);
    void addData(const QMap <QString, int> &data);
    void removeData(const QString &key);
    void updateView();

    QSizeF sizeHint(Qt::SizeHint, const QSizeF&) const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, 
                        QWidget *widget);

signals:
    // emitted when the geometry of the applet who contains the widget needs to be updated (new data to display ...)
    void geometryChanged();

private:
    class Private;
    Private * const d;
};

#endif

