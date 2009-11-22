/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *
 *   Copyright (C) 2008 - 2009 by Lukas Appelhans <l.appelhans@gmx.de>
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
#include "kgetapplet.h"
#include "kget_interface.h"
#include "kgetappletutils.h"

#include <plasma/dataengine.h>
#include <plasma/theme.h>
#include <plasma/widgets/iconwidget.h>
#include <plasma/widgets/meter.h>

#include <QGraphicsSceneDragDropEvent>
#include <QGraphicsLinearLayout>
#include <QDropEvent>
#include <QStyleOptionGraphicsItem>
#include <QProgressBar>
#include <QGraphicsProxyWidget>
#include <QPainter>
#include <QtDBus/QDBusConnectionInterface>
#include <KUrl>
#include <KLocale>
#include <KIcon>

const static int TOP_MARGIN = 55;
const static int LEFT_MARGIN = 15;
const static int SPACING = 4;
const static int MARGIN = 20;
const static int MAX_DOWNLOADS_PER_PAGE = 5;

ProxyWidget::ProxyWidget(QGraphicsWidget * parent) 
  : QGraphicsWidget(parent),
    m_layout(0),
    m_dataWidget(0)
{
    m_layout = new QGraphicsLinearLayout(Qt::Vertical, this);
    m_layout->setSpacing(SPACING);
    //m_layout->setContentsMargins(MARGIN, TOP_MARGIN, MARGIN, MARGIN);
    
    themeChanged();

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), SLOT(themeChanged()));
}

ProxyWidget::~ProxyWidget()
{
}

void ProxyWidget::paint(QPainter * p, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    QRect rect = option->rect;
    
    p->setRenderHint(QPainter::SmoothPixmapTransform);
    
    QFont font = Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont);
    font.setBold(true);
    font.setPointSize(15);

    p->setFont(font);
    p->setPen(Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));

    QRect iconRect(QPoint(rect.x() + SPACING + 10, rect.y() + SPACING + 10), QSize(m_textHeight, m_textHeight));
    
    KIcon("kget").paint(p, iconRect);
    p->drawText(QRectF(rect.x() + SPACING * 2 + 10 + iconRect.width(), rect.y() + SPACING + 10, 
                       m_textWidth, m_textHeight), i18n("KGet"));
    p->drawLine(QPointF(rect.x() + SPACING + 10, rect.y() + SPACING * 2 + 10 + m_textHeight), 
                QPointF(rect.width() - SPACING - 10, rect.y() + SPACING * 2 + 10 + m_textHeight));

    QGraphicsWidget::paint(p, option, widget);
}

void ProxyWidget::themeChanged()
{
    QFont font = Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont);
    font.setBold(true);
    font.setPointSize(15);

    QFontMetrics metrics(font);
    m_textWidth = metrics.width(i18n("KGet"));
    m_textHeight = metrics.height();
    
    m_layout->setContentsMargins(MARGIN, MARGIN + m_textHeight + SPACING + 10, MARGIN, MARGIN);
}

void ProxyWidget::setDataWidget(QGraphicsWidget *widget)
{
    if (m_layout->count())
        m_layout->removeAt(0);
    m_layout->addItem(widget);
    m_dataWidget = widget;
}

QGraphicsWidget * ProxyWidget::dataWidget()
{
    return m_dataWidget;
}

KGetApplet::KGetApplet(QObject *parent, const QVariantList &args)
  : Plasma::PopupApplet(parent, args),
    m_proxyWidget(0),
    m_errorWidget(0),
    m_dataWidget(0),
    m_icon(0),
    m_engine(0)
{
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    setBackgroundHints(Applet::DefaultBackground);
    setAcceptDrops(true);
    m_proxyWidget = new ProxyWidget(this);
}

KGetApplet::~KGetApplet()
{
    delete m_globalProgress;
}

void KGetApplet::init()
{
    KGlobal::locale()->insertCatalog("plasma_applet_kget");

    setPopupIcon("kget");
    m_engine = dataEngine("kget");
    if (m_engine) {
        m_engine->connectSource("KGet", this, 5000);
    } else {
        kDebug(5001) << "KGet Engine could not be loaded";
    }
    m_globalProgress = new Plasma::Meter(this);
    m_globalProgress->setMeterType(Plasma::Meter::BarMeterHorizontal);
    m_globalProgress->setMinimumSize(QSize(0, 0));
    setGraphicsWidget(m_proxyWidget);
}

void KGetApplet::setTransfers(const QVariantMap &transfers)
{
    QList<OrgKdeKgetTransferInterface*> ts;
    QList<OrgKdeKgetTransferInterface*> added;
    QVariantMap::const_iterator it;
    QVariantMap::const_iterator itEnd = transfers.constEnd();
    for (it = transfers.constBegin(); it != itEnd; ++it) {
        const QString &url = it.key();
        OrgKdeKgetTransferInterface* t = 0;
        foreach (OrgKdeKgetTransferInterface* transfer, m_transfers) {
            if (transfer->source().value() == url) {
                t = transfer;
                m_transfers.removeAll(transfer);
                break;
            }
        }
        if (t)
            ts.append(t);
        else {
            OrgKdeKgetTransferInterface* transfer = new OrgKdeKgetTransferInterface("org.kde.kget", 
                                                                                    transfers[url].toString(), QDBusConnection::sessionBus(), this);
            ts.append(transfer);
            added.append(transfer);
        }
    }
    if (!m_transfers.isEmpty())
        emit transfersRemoved(m_transfers);
    if (!added.isEmpty())
        emit transfersAdded(added);
    m_transfers = ts;
    emit update();
}

void KGetApplet::dataUpdated(const QString &name, const Plasma::DataEngine::Data &data)
{
    Q_UNUSED(name)
    kDebug() << layout()->count();
    if (data["error"].toBool()) {
        if (!m_errorWidget) {
            m_errorWidget = new ErrorWidget(data["errorMessage"].toString(), this);
        }
        if (m_proxyWidget->dataWidget() != m_errorWidget) {
            m_proxyWidget->setDataWidget(m_errorWidget);
            m_errorWidget->show();
            m_dataWidget->hide();
            setTransfers(QVariantMap());
        }
    }
    else if (!data["error"].toBool()) {
        if (m_errorWidget && m_errorWidget->isVisible())
            m_errorWidget->hide();
        if (m_proxyWidget->dataWidget() != m_dataWidget) {
            m_proxyWidget->setDataWidget(m_dataWidget);
            m_dataWidget->show();
        }
        setTransfers(data["transfers"].toMap());
        int completedSize = 0;
        int totalSize = 0;
        foreach (OrgKdeKgetTransferInterface* transfer, m_transfers) {
            completedSize += transfer->downloadedSize().value();
            totalSize += transfer->totalSize().value();
        }
        m_globalProgress->setValue((int) ((completedSize * 100) / totalSize));
    }
}

void KGetApplet::setDataWidget(QGraphicsWidget * widget)
{
    m_dataWidget = widget;
    if (m_proxyWidget->dataWidget() != m_errorWidget)
        m_proxyWidget->setDataWidget(widget);
}

void KGetApplet::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints & Plasma::SizeConstraint) {
        QGraphicsLayoutItem *widget = layout()->itemAt(0);
        if (dynamic_cast<Plasma::IconWidget*>(widget) && !m_icon)
            m_icon = dynamic_cast<Plasma::IconWidget*>(widget);
        if (widget == m_proxyWidget && m_globalProgress->isVisible()) {
            kDebug() << "remove progressbar";
            m_globalProgress->hide();
            dynamic_cast<QGraphicsLinearLayout*>(layout())->removeItem(m_globalProgress);
        } else if (m_icon && m_icon->isVisible()) {
            QGraphicsLinearLayout *lay = dynamic_cast<QGraphicsLinearLayout*>(layout());
            kDebug() << "switch to progressbar";
            m_globalProgress->show();
            m_icon->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
            m_icon->setPreferredSize(size().height(), size().height());
            m_globalProgress->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            for (int i = 0; i != 2; i++) {
                if (lay->count())
                    lay->removeAt(0);
            }
            lay->addItem(m_icon);
            lay->addItem(m_globalProgress);
        }
        setAspectRatioMode(Plasma::IgnoreAspectRatio);
        /*if (layout()->count() && dynamic_cast<Plasma::IconWidget*>(layout()->itemAt(0)) && !m_progressProxy.isVisible()) {
            qobject_cast<QGraphicsLinearLayout*>(layout())->addItem(m_progressProxy);
            m_progressProxy->show();
        } else if (m_progressProxy.isVisible() && layout->count() == 1) {
            kDebug();
            layout()->removeAt(0);
            m_progressProxy->hide();
        }*/
    }
}

bool KGetApplet::sceneEventFilter(QGraphicsItem * watched, QEvent * event)
{
    Q_UNUSED(watched)
    switch (event->type())
    {
        case QEvent::GraphicsSceneDrop:
            dropEvent(static_cast<QGraphicsSceneDragDropEvent*>(event));
            return true;
        case QEvent::Drop:
            dropEvent(static_cast<QDropEvent*>(event));
            return true;
        default:
            break;
    }
    return false;
}

void KGetApplet::dropEvent(QGraphicsSceneDragDropEvent * event)
{
    kDebug(5001);    
    
    QStringList urls;
    if (event->mimeData()->hasUrls())
    {
        foreach (const KUrl &url, event->mimeData()->urls())
            urls.append(url.url());
    }
    else
    {
        event->ignore();
        return;
    }
    
    if (QDBusConnection::sessionBus().interface()->isServiceRegistered(KGET_DBUS_SERVICE))
    {
        OrgKdeKgetMainInterface kget_interface(KGET_DBUS_SERVICE, KGET_DBUS_PATH,
                            QDBusConnection::sessionBus());

        kget_interface.showNewTransferDialog(urls);
    }
    else
    {
        QProcess::startDetached("kget", urls);
    }
    event->accept();
}

void KGetApplet::dropEvent(QDropEvent * event)
{
    kDebug(5001);
    
    QStringList urls;
    if (event->mimeData()->hasUrls())
    {
        foreach (const KUrl &url, event->mimeData()->urls())
            urls.append(url.url());
    }
    else
    {
        event->ignore();
        return;
    }
    
    if (QDBusConnection::sessionBus().interface()->isServiceRegistered(KGET_DBUS_SERVICE))
    {
        OrgKdeKgetMainInterface kget_interface(KGET_DBUS_SERVICE, KGET_DBUS_PATH,
                            QDBusConnection::sessionBus());

        kget_interface.showNewTransferDialog(urls);
        event->accept();
    }
    else
    {
        QProcess::startDetached("kget", urls);
    }
    event->accept();
}

#include "kgetapplet.moc"
