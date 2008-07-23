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

#include "kgetappletutils.h"

#include <plasma/svg.h>
#include <plasma/widgets/label.h>
#include <plasma/widgets/icon.h>
#include <plasma/widgets/pushbutton.h>

#include <QGraphicsLinearLayout>
#include <QPainter>
#include <QRect>
#include <QVBoxLayout>
#include <QLabel>
#include <QProcess>
#include <QPushButton>

#include <KIcon>
#include <KLocale>
#include <KPushButton>

const static int SPACING = 4;


void KGetAppletUtils::paintTitle(QPainter *p, Plasma::Svg *svg, const QRect &rect)
{
    p->setRenderHint(QPainter::SmoothPixmapTransform);

    svg->paint(p, QRect(rect.x() + SPACING + 10,
                        rect.y() + SPACING + 10, 111, 35), "title");
    svg->paint(p, QRect(rect.x() + SPACING + 10,
                        rect.y() + SPACING + 45,
                        rect.width() - (SPACING + 10) * 2, 1), "line");
}

QGraphicsWidget *KGetAppletUtils::createErrorWidget(const QString &message, QGraphicsWidget *parent)
{
    return new ErrorWidget(message, parent);
}


/** Error widget **/

ErrorWidget::ErrorWidget(const QString &message, QGraphicsWidget *parent)
    : QGraphicsProxyWidget(parent)
{
    m_layout = new QGraphicsLinearLayout(this);
    m_layout->setOrientation(Qt::Vertical);

    m_errorLabel = new Plasma::Label(this);
    m_errorLabel->setText(message);
    m_errorLabel->nativeWidget()->setAlignment(Qt::AlignCenter);

    m_icon = new Plasma::Icon(KIcon("dialog-warning"),"", this);

    m_launchButton = new Plasma::PushButton(this);
    m_launchButton->setText(i18n("Launch KGet"));
    m_launchButton->nativeWidget()->setIcon(KIcon("kget"));

    m_layout->addItem(m_errorLabel);
    m_layout->addItem(m_icon);
    m_layout->addItem(m_launchButton);

    setLayout(m_layout);

    connect(m_launchButton, SIGNAL(clicked()), SLOT(launchKGet()));
}

ErrorWidget::~ErrorWidget()
{
    delete m_errorLabel;
    delete m_icon;
    delete m_launchButton;
}

void ErrorWidget::launchKGet()
{
    QProcess kgetProcess;
    kgetProcess.startDetached("kget");
}

#include "kgetappletutils.moc"
