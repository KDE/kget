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

#include "linegraphwidget.h"

#include <QMap>
#include <QQueue>
#include <QPainter>
#include <QFontMetrics>
#include <QStyleOptionGraphicsItem>

#include <KDebug>
#include <KGlobal>
#include <KLocale>
#include <KColorCollection>

#include <plasma/layouts/layout.h>

#define TOP_MARGIN 10
#define LEFT_MARGIN 75
#define RIGHT_MARGIN 10
#define MAX_ITEMS 30
#define MAX_Y_STEPS 10
#define MAX_X_STEPS 15

class LineGraphWidget::Private : public Plasma::Widget
{
public:
    Private(Plasma::Widget *parent) : Plasma::Widget(parent),
            m_colors("Oxygen.colors"),
            size(QSize(300, 180)),
            maximumY(20 * 1024),
            minimumY(0),
            bottomMargin(10)
    {
        setGeometry(QRect(LEFT_MARGIN, TOP_MARGIN,
                                    size.width() - LEFT_MARGIN - RIGHT_MARGIN,
                                    size.height() - TOP_MARGIN - 2));
    }

    // Paint the private widget, who cares to draw only the line items representing the data
    void paintWidget(QPainter *p, const QStyleOptionGraphicsItem *option,
                        QWidget *widget)
    {
        Q_UNUSED(option)
        Q_UNUSED(widget)

        kDebug() << "About to repaint the inner widget" << endl;

        if(data.size() > 0) {
            for(int i = 0; i < data.size(); i++) {
                drawQueue(data [data.keys().at(i)], p, m_colors.color(i*6 + 4));
            }
        }
        else {
            p->setPen(Qt::white);
            p->drawText(QRect(10, size.height()/4, 200, 20), Qt::AlignCenter, i18n("n/a"));
        }
    }

    // Draw the graph legend with the names of the data
    void drawLegend(const QString &title, QPainter *p, const QColor &color, int count)
    {
        p->save();
        p->setPen(Qt::NoPen);
        p->setBrush(QBrush(color));
        p->drawRoundRect(QRect(LEFT_MARGIN, count * 20 + size.height() - bottomMargin + TOP_MARGIN, 10, 10));
        p->setPen(Qt::SolidLine);
        p->setPen(Qt::white);

        // the data name
        p->drawText(QRect(LEFT_MARGIN + 14, count * 20 + size.height () - bottomMargin + TOP_MARGIN - 5, 180, 20), Qt::AlignLeft, p->fontMetrics().elidedText(title, Qt::ElideLeft, 180));
        p->restore();
    }

    // Draw the graph's axis
    void drawAxis(QPainter *p, const QStyleOptionGraphicsItem *option)
    {
        Q_UNUSED(option)

        kDebug() << "About to draw the axis";

        // draw the y axis 0..maxY
        p->setPen(Qt::white);
        p->drawLine(QPoint(0 + LEFT_MARGIN, TOP_MARGIN),
                    QPoint(0 + LEFT_MARGIN, size.height() - bottomMargin));

        // draw the x axis
        p->drawLine(QPoint(0 + LEFT_MARGIN, size.height() - bottomMargin),
                    QPoint(size.width() - RIGHT_MARGIN, size.height() - bottomMargin));

        p->save();
        p->setOpacity(0.10);

        // draw the y line items
        for(int i=0; i< MAX_Y_STEPS; i++) {
            int y = (i * ((size.height() - bottomMargin - TOP_MARGIN) / MAX_Y_STEPS)) + TOP_MARGIN;

            p->drawLine(QPoint(LEFT_MARGIN, y),
                    QPoint(size.width() - RIGHT_MARGIN, y));

            p->save();
            p->setOpacity(0.40);
            p->drawText(2, y + 4,
                            KGlobal::locale()->formatByteSize(((MAX_Y_STEPS - i) * maximumY) / 10));
            p->restore();
        }

        // draw the x line items
        for(int i=0; i<MAX_X_STEPS; i++) {
            int x = ((i + 1) * (size.width() - LEFT_MARGIN - RIGHT_MARGIN) / MAX_X_STEPS) + LEFT_MARGIN;

            p->drawLine(QPoint(x, TOP_MARGIN),
                        QPoint(x, size.height() - bottomMargin));
        }

        p->restore();
    }

    // returns the size of the private inner widget
    QSizeF sizeHint() const
    {
        return QSizeF(size.width() - LEFT_MARGIN - RIGHT_MARGIN, 170);
    }

    KColorCollection m_colors;
    QSize size;
    QMap <QString, QQueue <int> > data;
    int maximumY;
    int minimumY;
    int bottomMargin;

private:

    // Draws the entire line of one item's data
    void drawQueue(QQueue <int> queue, QPainter *p, const QColor &color)
    {
        p->save();
        p->setPen(color);

        QPoint previous;

        for(int i = 0; i < queue.size(); i++) {
            int x = i * ((size.width() - RIGHT_MARGIN) / MAX_X_STEPS) +  1;
            int y = (maximumY - queue.at(queue.size() - 1 -i)) * (size.height() - bottomMargin) / maximumY;
            y -= TOP_MARGIN;

            p->drawPoint(x, y);

            if(!previous.isNull())
                p->drawLine(x, y, previous.x(), previous.y());

            previous.setX(x);
            previous.setY(y);
        }

        p->restore();
    }
};

LineGraphWidget::LineGraphWidget(Widget *parent)
    : Plasma::Widget(parent),
    d(new Private(this))
{
}

LineGraphWidget::~LineGraphWidget()
{
    delete d;
}

void LineGraphWidget::addData(const QString &key, int data)
{
    if(!d->data.contains(key)) {
        d->data [key] = QQueue <int> ();

        d->bottomMargin += 20;
        d->size.setHeight (d->size.height() + 20);

        emit geometryChanged();
    }

    if(d->data [key].size() >= MAX_ITEMS) {
        d->data [key].dequeue();
    }

    // TODO: if the maximum bound is reached then update the maxiumumY and updateGeometry() to repaint the axis
    if (data > d->maximumY) {
        d->maximumY = data + 20 * 1024;
        updateGeometry();
    }
    d->data [key].enqueue(data);
}

void LineGraphWidget::addData(const QMap <QString, int> &data)
{
    foreach(QString key, data.keys()) {
        if(!d->data.contains(key)) {
            d->data [key] = QQueue <int> ();
            d->bottomMargin += 20;
            d->size.setHeight (d->size.height() + 20);

            emit geometryChanged();
        }

        if(d->data [key].size() >= MAX_ITEMS) {
            d->data [key].dequeue();
        }

        d->data [key].enqueue(data[key]);

        if (data [key] > d->maximumY) {
            d->maximumY = data [key] + 20 * 1024;
            updateGeometry();
        }
    }
}

void LineGraphWidget::removeData(const QString &key)
{
    d->data.remove(key);
    emit geometryChanged();
}

void LineGraphWidget::updateView()
{
    // ensure that only the data points are repainted, called after the setData methods
    d->updateGeometry();
}

QSizeF LineGraphWidget::sizeHint() const
{
    return d->size;
}

void LineGraphWidget::paintWidget(QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    d->drawAxis(p, option);
    for(int i = 0; i < d->data.size(); i++) {
        d->drawLegend(d->data.keys().at(i), p, d->m_colors.color(i*6 + 4), i);
    }
}
