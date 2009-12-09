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
#ifndef KGETPIECHART_P_H
#define KGETPIECHART_P_H

#include <QtCore/QHash>
#include <QtGui/QGraphicsWidget>
#include <QtGui/QPen>

#include <KColorCollection>
#include <KIO/Job>

class QGraphicsLinearLayout;

namespace Plasma
{
    class Label;
    class ScrollWidget;
}

class KGetPieChart::Data
{
    public:
        QString name;
        bool isFinished;
        KIO::filesize_t size;
        KIO::filesize_t downloadedSize;
        QColor color;
};

class KGetPieChart::PieChart : public QGraphicsWidget
{
    Q_OBJECT

    public:
        PieChart(QHash<OrgKdeKgetTransferInterface*, Data> *data, KIO::filesize_t totalSize, QGraphicsWidget *parent = 0);
        ~PieChart();

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
        void setTotalSize(KIO::filesize_t totalSize);
        void createAngles();

    private:
        QHash<OrgKdeKgetTransferInterface*, Data> *m_data;
        KIO::filesize_t m_totalSize;
        QHash<OrgKdeKgetTransferInterface*, QPair<int, int> > m_angles;
        QPen m_totalPen;
        QPen m_activePen;

        static const float PIE_OPACITY;
        static const float ACTIVE_PIE_OPACITY;
};

class KGetPieChart::Item : public QGraphicsWidget
{
    Q_OBJECT

    public:
        Item(QGraphicsWidget *parent = 0);
        ~Item();

        void setSize(KIO::filesize_t size);
        void setName(const QString &name);
        void setColor(const QColor &color);

    private:
        Plasma::Label *m_name;
        Plasma::Label *m_sizeLabel;
        Plasma::Label *m_colorLabel;
};

class KGetPieChart::Private : public QGraphicsWidget
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
        void updateTransfers();

    private:
        KColorCollection m_colors;
        KIO::filesize_t m_totalSize;
        Plasma::ScrollWidget *m_scrollWidget;
        QGraphicsWidget *m_containerWidget;
        QGraphicsLinearLayout *m_containerLayout;
        QHash<OrgKdeKgetTransferInterface*, Data> m_data;
        QHash<OrgKdeKgetTransferInterface*, Item*> m_items;
        PieChart *m_piechart;
};

#endif
