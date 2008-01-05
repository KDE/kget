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

#include "piechartwidget.h"

#include <math.h>

#include <QPainter>
#include <QStyleOptionGraphicsItem>

#include <KDebug>
#include <KLocale>
#include <KGlobal>
#include <KColorCollection>

#define TOP_MARGIN 5
#define LEFT_MARGIN 15
#define PIE_SIZE 140

class PieChartWidget::PrivateData
{
public:
    PrivateData()
    {}

    QString name;
    bool isActive;
    float length;
    float activeLength;
};

class PieChartWidget::Private : public Plasma::Widget
{
public:
    Private(Plasma::Widget *parent) : Plasma::Widget(parent),
                m_colors("Oxygen.colors"),
                size(QSize(300, 200)),
                bottomMargin(10),
                totalSize(0),
                emitUpdateGeometrySignal(false)
    {
        setGeometry(QRect(0, TOP_MARGIN, PIE_SIZE + 2, PIE_SIZE + 2));
    }

    void resize(int width, int height)
    {
        size.setWidth(width);
        size.setHeight(height);
        setGeometry(QRect(width/4, TOP_MARGIN, width/2, height/2));
    }

    // returns the size of the private inner widget
    QSizeF sizeHint() const
    {
        return size;
    }

    inline double roundNumber (float number)
    {
        double intPart;
        if (modf(number, &intPart) > 0.5) {
            return intPart + 1;
        }
        else {
            return intPart;
        }
    }

    // Paint the private widget, who cares to draw only the line items representing the data
    void paintWidget(QPainter *p, const QStyleOptionGraphicsItem *option,
                        QWidget *widget)
    {
        Q_UNUSED(widget);
        Q_UNUSED(option);

        if(totalSize > 0) {
            kDebug() << "About to repaint the inner widget" << endl;

            p->save();
            p->setRenderHint(QPainter::Antialiasing);

            QRect rect(1, 1, size.width()/2 - 2, size.width()/2 - 2);
            int angle = 90 * 16; // 0

            QPen totalPen;
            QPen activePen;

            // total pen
            totalPen.setWidth(2);
            totalPen.setColor(Qt::darkGray);
            totalPen.setStyle(Qt::SolidLine);

            activePen.setWidth(2);
            activePen.setColor(Qt::white);
            activePen.setStyle(Qt::SolidLine);

            for(int i = 0; i < data.size(); i++) {
                QBrush brush(m_colors.color(i*6 + 4));

                PrivateData portion = data [data.keys().at(i)];

                float pieOpacity = 0.67;
                float activePieOpacity = 0.84;
                int current_angle = angle;

                p->save();

                if (portion.isActive) {
                    p->setPen(activePen);
                }
                else {
                    p->setPen(totalPen);
                }

                p->setOpacity(pieOpacity);

                int totalPercent = (int) roundNumber(portion.length * 100/ totalSize);
                angle = drawPie(p, rect, angle, totalPercent, brush);
                p->restore();

                p->save();
                // draw the active length
                p->setOpacity(activePieOpacity);
                p->setPen(Qt::NoPen);
                int activePercent = (int) roundNumber(portion.activeLength * 100/ totalSize);
                drawPie(p, QRect(rect.x() + 15, rect.y() + 15, rect.width() - 30, rect.height() - 30),
                                current_angle, activePercent, brush);

                p->restore();
            }

            p->restore();
        }
        else {
            p->setPen(Qt::white);
            p->drawText(QRect(2, 2, size.width()/2 -2, size.width()/2 - 2), Qt::AlignCenter, i18n("n/a"));
        }
    }

    int drawPie(QPainter *p, const QRect &rect, int angle, int percent, const QBrush &brush)
    {
        int end_angle = -1 * percent * 36/10 * 16;

        p->setBrush(brush);
        p->drawPie(rect, angle, end_angle);

        return end_angle + angle;
    }

    // Draw the graph legend with the names of the data
    void drawLegend(const QString &name, QPainter *p, const QStyleOptionGraphicsItem *option, const QColor &color, int count)
    {
        int textLength = option->rect.width() - 100;
        p->save();
        p->setPen(Qt::NoPen);
        p->setBrush(QBrush(color));
        p->drawRoundRect(QRect(LEFT_MARGIN, count * 20 + option->rect.height() - bottomMargin + TOP_MARGIN, 10, 10));
        p->setPen(Qt::SolidLine);
        p->setPen(Qt::white);

        // the data name
        p->drawText(QRect(LEFT_MARGIN + 14, count * 20 + option->rect.height () - bottomMargin + TOP_MARGIN - 5, textLength, 20), Qt::AlignLeft, p->fontMetrics().elidedText(name, Qt::ElideLeft, textLength));
        // the data percent
        p->drawText(QRect(LEFT_MARGIN + 14 + textLength, count * 20 + option->rect.height () - bottomMargin + TOP_MARGIN - 5, 150, 20), Qt::AlignLeft,
        KGlobal::locale()->formatByteSize(data [name].length));

        p->restore();
    }

    KColorCollection m_colors;
    QSize size;
    QMap <QString, PrivateData> data;
    int bottomMargin;
    float totalSize;
    bool emitUpdateGeometrySignal;
};

PieChartWidget::PieChartWidget(Widget *parent)
    : Plasma::Widget(parent),
    d(new Private(this))
{
}

PieChartWidget::~PieChartWidget()
{
    delete d;
}

void PieChartWidget::addData(QString name, int length)
{
    addData(name, length, 0, false);
}

void PieChartWidget::addData(QString name, int length, int activeLength)
{
    addData(name, length, activeLength, false);
}

void PieChartWidget::addData(QString name, int length, int activeLength, bool active)
{
    if (!d->data.contains(name)) {
        d->data [name] = PrivateData ();

        // update the widget size
        d->bottomMargin += 20;
        d->size.setHeight (d->size.height() + 20);

        d->emitUpdateGeometrySignal = true;
    }

    d->data [name].name = name;
    d->data [name].length = length;
    d->data [name].activeLength = activeLength;
    d->data [name].isActive = active;

    d->totalSize += length;
}

void PieChartWidget::removeData(const QString &key)
{
    d->data.remove(key);
    emit geometryChanged();
}

void PieChartWidget::clear()
{
    d->totalSize = 0;
    d->emitUpdateGeometrySignal = false;
}

void PieChartWidget::updateView()
{
    // ensure that only the data points are repainted, called after the setData methods
    if (d->emitUpdateGeometrySignal) {
       d->emitUpdateGeometrySignal = false;
       emit geometryChanged();
    }
    d->updateGeometry();
}

QSizeF PieChartWidget::sizeHint() const
{
    return d->size;
}

void PieChartWidget::paintWidget(QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)

    d->resize(option->rect.width(), option->rect.height());

    kDebug() << "About to draw the parent widget" ;

    for(int i = 0; i < d->data.size(); i++) {
        d->drawLegend(d->data.keys().at(i), p, option, d->m_colors.color(i*6 + 4), i);
    }
}
