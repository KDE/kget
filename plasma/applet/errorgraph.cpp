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

#include <QGraphicsLinearLayout>
#include <QGraphicsProxyWidget>
#include <QLabel>
#include <QProcess>
#include <QPushButton>

#include <KIcon>

ErrorGraph::ErrorGraph(Plasma::Applet *parent, const QString &message)
    : TransferGraph(parent)
{
    m_layout = static_cast <QGraphicsLinearLayout *> (parent->layout());
    if (m_layout)
    {
        m_icon = new Plasma::Icon(KIcon("dialog-warning"), "");

        QLabel *errorLabel = new QLabel();
        errorLabel->setAutoFillBackground(true);
        errorLabel->setText(message);
        //errorLabel->setPen(QPen(Qt::white));
        //errorLabel->setAlignment(Qt::AlignCenter);

        QPushButton *launchButton = new QPushButton(KIcon("kget"), "Launch KGet");
        launchButton->setAutoFillBackground(true);

        m_proxyErrorLabel = new QGraphicsProxyWidget(parent);
        m_proxyErrorLabel->setWidget(errorLabel);

        m_proxyLaunchButton = new QGraphicsProxyWidget(parent);
        m_proxyLaunchButton->setWidget(launchButton);

        m_layout->addItem(m_icon);
        m_layout->addItem(m_proxyErrorLabel);
        m_layout->addItem(m_proxyLaunchButton);

        connect(launchButton, SIGNAL(clicked()), SLOT(launchKGet()));
    }
}

ErrorGraph::~ErrorGraph()
{
    m_layout->removeItem(m_icon);
    m_layout->removeItem(m_proxyErrorLabel);
    m_layout->removeItem(m_proxyLaunchButton);

    m_proxyErrorLabel->setWidget(0);
    m_proxyLaunchButton->setWidget(0);

    delete m_proxyErrorLabel;
    delete m_proxyLaunchButton;
    delete m_icon;
}

void ErrorGraph::launchKGet()
{
    QProcess *kgetProcess = new QProcess(this);
    kgetProcess->startDetached("kget");
}

