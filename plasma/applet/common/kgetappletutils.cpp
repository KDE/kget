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

#include "kgetappletutils.h"

#include <plasma/svg.h>
#include <plasma/theme.h>
#include <plasma/widgets/label.h>
#include <plasma/widgets/iconwidget.h>
#include <plasma/widgets/pushbutton.h>

#include <QtDBus/QDBusConnectionInterface>
#include <QGraphicsLinearLayout>
#include <QPainter>
#include <QRect>
#include <QVBoxLayout>
#include <QLabel>
#include <QProcess>
#include <QPushButton>
#include <QTimer>

#include <KIcon>
#include <KLocale>
#include <KPushButton>

const int KGetAppletUtils::SPACING = 4;


void KGetAppletUtils::paintTitle(QPainter *p, Plasma::Svg *svg, const QRect &rect)
{
    Q_UNUSED(svg)

    p->setRenderHint(QPainter::SmoothPixmapTransform);
    QFont font = Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont);
    font.setBold(true);
    font.setPointSize(15);
    QFontMetrics metrics(font);
    p->setFont(font);
    p->setPen(Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));

    QRect iconRect(QPoint(rect.x() + SPACING + 10, rect.y() + SPACING + 10), QSize(metrics.height(), metrics.height()));
    KIcon("kget").paint(p, iconRect);
    //p->drawPixmap(QPointF(rect.x() + SPACING + 10, rect.y() + SPACING + 10), KIcon("kget").pixmap(iconRect.width(), iconRect.height()), iconRect);
    //svg->paint(p, QRect(rect.x() + SPACING + 10,
    //                    rect.y() + SPACING + 10, 111, 35), "title");
    //p->setPen(Qt::black);
    p->drawText(QRectF(rect.x() + SPACING * 2 + 10 + iconRect.width(), rect.y() + SPACING + 10, 
                       metrics.width(i18n("KGet")), metrics.height()), i18n("KGet"));
    p->drawLine(QPointF(rect.x() + SPACING + 10, rect.y() + SPACING * 2 + 10 + metrics.height()), 
                QPointF(rect.width() - SPACING - 10, rect.y() + SPACING * 2 + 10 + metrics.height()));
    //svg->paint(p, QRect(rect.x() + SPACING + 10,
    //                    rect.y() + SPACING + 45,
    //                    rect.width() - (SPACING + 10) * 2, 1), "line");
}

QGraphicsWidget *KGetAppletUtils::createErrorWidget(const QString &message, QGraphicsWidget *parent)
{
    return new ErrorWidget(message, parent);
}


/** Error widget **/

ErrorWidget::ErrorWidget(const QString &message, QGraphicsWidget *parent)
    : QGraphicsProxyWidget(parent)
{
    m_interface = QDBusConnection::sessionBus().interface();

    m_layout = new QGraphicsLinearLayout(this);
    m_layout->setOrientation(Qt::Vertical);

    m_errorLabel = new Plasma::Label(this);
    m_errorLabel->setText(message);
    m_errorLabel->nativeWidget()->setAlignment(Qt::AlignCenter);

    m_icon = new Plasma::IconWidget(KIcon("dialog-warning"),"", this);

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
    checkKGetStatus();
}

void ErrorWidget::checkKGetStatus()
{
    if (m_interface->isServiceRegistered("org.kde.kget")) {
        emit kgetStarted();
    } else {
        QTimer::singleShot(1000, this, SLOT(checkKGetStatus()));
    }
}

#include "kgetappletutils.moc"
