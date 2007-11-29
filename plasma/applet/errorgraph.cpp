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

#include "errorgraph.h"
#include "transfergraph.h"

#include <plasma/widgets/icon.h>
#include <plasma/widgets/label.h>
#include <plasma/layouts/boxlayout.h>
#include <plasma/widgets/pushbutton.h>

#include <KIcon>
#include <QProcess>

ErrorGraph::ErrorGraph(Plasma::Applet *parent, Plasma::BoxLayout *mainlayout, const QString &message)
    : TransferGraph(parent)
{
    m_layout = mainlayout;

    m_errorLabel = new Plasma::Label(m_applet);
    m_errorLabel->setText(message);
    m_errorLabel->setPen(QPen(Qt::white));
    m_errorLabel->setAlignment(Qt::AlignLeft);
    m_launchButton = new Plasma::PushButton(KIcon("kget"), "Launch KGet", m_applet);

    m_layout->addItem(m_errorLabel);
    m_layout->addItem(m_launchButton);

    m_applet->resize(QSize(500, 700));

    connect(m_launchButton, SIGNAL(clicked()), SLOT(launchKGet()));
}

ErrorGraph::~ErrorGraph()
{
    delete m_errorLabel;
    delete m_launchButton;
}

void ErrorGraph::launchKGet()
{
    QProcess *kgetProcess = new QProcess(this);
    kgetProcess->startDetached("kget");
}
