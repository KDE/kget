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

#ifndef ERRORGRAPH_H
#define ERRORGRAPH_H

#include "transfergraph.h"

namespace Plasma {
    class Applet;
    class Icon;
}

class QGraphicsLinearLayout;
class QGraphicsProxyWidget;

class ErrorGraph : public TransferGraph
{
    Q_OBJECT
public:
    ErrorGraph(Plasma::Applet *parent, const QString &message);
    ~ErrorGraph();

private slots:
    void launchKGet();

private:
    QGraphicsLinearLayout *m_layout;
    QGraphicsProxyWidget *m_proxyErrorLabel;
    QGraphicsProxyWidget *m_proxyLaunchButton;
    Plasma::Icon *m_icon;
};

#endif

