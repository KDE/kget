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

#ifndef BARCHART_H
#define BARCHART_H

#include "transfergraph.h"
#include <QMap>

#define MAX_DOWNLOADS_PER_PAGE 5

class QGraphicsLinearLayout;
class QGraphicsProxyWidget;
class QGraphicsWidget;
class QLabel;
class QProgressBar;
class QPushButton;
class QString;
class QVBoxLayout;
class QWidget;

class BarChart : public TransferGraph
{
Q_OBJECT

public:
    BarChart(QGraphicsWidget *parent);
    ~BarChart();

    void setTransfers(const QVariantMap &transfers);

public slots:
    void nextPage();
    void previousPage();

private slots:
    void populate();

private:
    void clear();

private:
    QGraphicsLinearLayout *m_layout;
    QVBoxLayout *m_verticalLayout;
    QGraphicsProxyWidget *m_proxyMainWidget;

    QWidget *m_mainWidget;
    QPushButton *m_nextPageButton;
    QPushButton *m_previousPageButton;
    QLabel *m_totalSizeLabel;
    QLabel *m_pageLabel;
    QMap <QString, QProgressBar *> m_progressBars;

    int m_actualPage;
};

#endif
