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

namespace Plasma {
    class BoxLayout;
    class ProgressBar;
    class Label;
    class PushButton;
}
class QString;

class BarChart : public TransferGraph
{
Q_OBJECT

public:
    BarChart(Plasma::Applet *parent);
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
    Plasma::BoxLayout *m_layout;
    Plasma::BoxLayout *m_pagerLayout;
    Plasma::PushButton *m_nextPageButton;
    Plasma::PushButton *m_previousPageButton;
    Plasma::Label *m_totalSizeLabel;
    Plasma::Label *m_pageLabel;
    QMap <QString, Plasma::ProgressBar *> m_progressBars;

    int m_actualPage;
};

#endif
