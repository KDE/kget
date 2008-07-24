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

#include "panelbar/kgetpanelbar.h"
#include "panelbar/kgetpanelbar_p.h"
#include "common/kgetappletutils.h"

#include <QGridLayout>
#include <QGraphicsProxyWidget>
#include <QGraphicsLinearLayout>
#include <QLabel>
#include <QPainter>
#include <QProgressBar>

#include <KDebug>
#include <KIcon>
#include <KIconLoader>
#include <KLocale>
#include <KTitleWidget>
#include <kio/global.h>

#include <plasma/applet.h>
#include <plasma/dataengine.h>
#include <plasma/dialog.h>
#include <plasma/svg.h>
#include <plasma/theme.h>
#include <plasma/widgets/icon.h>

const static int TOP_MARGIN = 60;
const static int SPACING = 4;
const static int MARGIN = 20;
const static int MAX_DOWNLOADS_PER_PAGE = 5;

KGetPanelBar::Private::Private(QGraphicsWidget *parent) : QGraphicsProxyWidget(parent),
    m_activeTransfers(),
    m_activeBars(),
    m_widgets()
{
    m_bar = new QProgressBar();

    m_bar->setValue(0);
    m_bar->setStyleSheet("background-color: transparent");

    setWidget(m_bar);

    m_dialogLayout = new QGridLayout(0);
    m_dialogLayout->setColumnStretch(0, 0);
    m_dialogLayout->setColumnStretch(1, 0);
    m_dialogLayout->setColumnStretch(2, 1);

    KTitleWidget *title = new KTitleWidget();
    title->setText(i18n("KGet Active transfers"));
    title->setPixmap(KIcon("kget").pixmap(22, 22), KTitleWidget::ImageRight);
    m_dialogLayout->addWidget(title, 0, 0, 1, 3);
}

KGetPanelBar::Private::~Private()
{
    setWidget(0);

    delete m_bar;
}

void KGetPanelBar::Private::setTransfers(const QVariantMap &transfers)
{
    double totalSize = 0;
    double completedSize = 0;

    m_transfers = transfers;

    // remove the progressbars for the deleted transfers
    foreach(const QString &key, m_activeTransfers.keys()) {
        if(!m_transfers.keys().contains(key)) {
            clear();
        }
    }

    foreach(const QString &key, transfers.keys()) {
        QVariantList attributes = transfers [key].toList();

        // only show the percent of the active transfers
        if(attributes.at(3).toUInt() == 1) {
            totalSize += attributes.at(2).toDouble();
            completedSize += ((attributes.at(1).toDouble() * attributes.at(2).toDouble()) / 100);

            showActiveTransfer(key, attributes);
       }
    }

    if(totalSize > 0) {
        m_bar->setValue((int) ((completedSize * 100) / totalSize));
    }
    else {
        m_bar->setValue(0);
    }
}

void KGetPanelBar::Private::showActiveTransfer(const QString &key, const QVariantList &attributes)
{
    if (m_activeTransfers.contains(key)) {
        int row = m_activeTransfers [key];
        QProgressBar *progressBar = m_activeBars [row];
        progressBar->setValue(attributes.at(1).toString().toInt());
    }
    else {
        QLabel *icon = new QLabel();
        QLabel *name = new QLabel();
        QProgressBar *progressBar = new QProgressBar();

        m_widgets << icon;
        m_widgets << name;
        m_widgets << progressBar;

        int row = m_dialogLayout->rowCount();
        m_activeTransfers [key] = row;
        m_activeBars [row] = progressBar;

        icon->setPixmap(KIO::pixmapForUrl(attributes.at(0).toString(), 0, KIconLoader::Desktop, 16));
        name->setText(attributes.at(0).toString());
        name->setStyleSheet(QString("color: %1;").
                            arg(Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor).name()));
        progressBar->setValue(attributes.at(1).toString().toInt());

        m_dialogLayout->addWidget(icon, row, 0);
        m_dialogLayout->addWidget(name, row, 1);
        m_dialogLayout->addWidget(progressBar, row, 2);
    }
}

void KGetPanelBar::Private::clear()
{
    m_activeTransfers.clear();
    m_activeBars.clear();

    foreach(QWidget *widget, m_widgets) {
        delete widget;
    }

    m_widgets.clear();

    // TODO : Find another way to remove all layout widgets except the title one
    for(int row=1; row < m_dialogLayout->rowCount(); row++) {
        delete m_dialogLayout->itemAtPosition(row, 0);
        delete m_dialogLayout->itemAtPosition(row, 1);
        delete m_dialogLayout->itemAtPosition(row, 2);

        m_dialogLayout->removeItem(m_dialogLayout->itemAtPosition(row, 0));
        m_dialogLayout->removeItem(m_dialogLayout->itemAtPosition(row, 1));
        m_dialogLayout->removeItem(m_dialogLayout->itemAtPosition(row, 2));
    }
}


KGetPanelBar::KGetPanelBar(QObject *parent, const QVariantList &args)
        : KGetApplet(parent, args),
        m_dialog(0),
        m_icon(0),
        d(new KGetPanelBar::Private(this))
{
    setBackgroundHints(Applet::NoBackground);
}

KGetPanelBar::~KGetPanelBar()
{
}

void KGetPanelBar::init()
{
    m_dialog = new Plasma::Dialog(0);
    m_dialog->setFocusPolicy(Qt::NoFocus);
    m_dialog->setWindowFlags(Qt::Popup);
    m_dialog->setLayout(d->dialogLayout());

    m_icon = new Plasma::Icon(KIcon("go-down"), QString(), this);

    m_layout = new QGraphicsLinearLayout(Qt::Horizontal, this);
    m_layout->addItem(m_icon);
    m_layout->addItem(d);

    setLayout(m_layout);

    connect(m_icon, SIGNAL(clicked()), SLOT(showDialog()));

    KGetApplet::init();
}

void KGetPanelBar::paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option, const QRect &contentsRect)
{
    Q_UNUSED(p)
    Q_UNUSED(option)
    Q_UNUSED(contentsRect)
}

void KGetPanelBar::dataUpdated(const QString &source, const Plasma::DataEngine::Data &data)
{
    Q_UNUSED(source)

    if(data["error"].toBool()) {
        kDebug() << "Error : " << data["errorMessage"].toString();
        d->setTransfers(QVariantMap());
    }
    else if(!data["error"].toBool()) {
        if (!d) {
            d = new KGetPanelBar::Private(this);
            m_layout->addItem(d);
        }

        d->setTransfers(data["transfers"].toMap());
    }
}

void KGetPanelBar::showDialog()
{
    if (m_dialog->isVisible()) {
        m_dialog->hide();
    }
    else {
        m_dialog->show();
        m_dialog->move(popupPosition(m_dialog->sizeHint()));
    }
}

#include "kgetpanelbar.moc"
#include "kgetpanelbar_p.moc"
