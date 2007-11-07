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
    class HBoxLayout;
    class PushButton;
    class Applet;
    class Label;
    class Icon;
}

class ErrorGraph : public TransferGraph
{
    Q_OBJECT
public:
    ErrorGraph(Plasma::Applet *parent, const QString &message);
    ~ErrorGraph();

    QSizeF contentSizeHint();

private slots:
    void launchKGet();

private:
    Plasma::HBoxLayout *m_layout;
    Plasma::Label *m_errorLabel;
    Plasma::PushButton *m_launchButton;
    Plasma::Icon *m_icon;
};

#endif
